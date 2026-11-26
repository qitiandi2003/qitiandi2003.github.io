// g++ -std=c++17 -O2 _qi-blog/build-plush.cpp $(pkg-config --cflags --libs opencv4 libavif) -o /tmp/build-plush
// /tmp/build-plush poses.png actions.png assets/pet-plush-hd.svg
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <avif/avif.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {
constexpr int w = 144, h = 168, columns = 16, transitionSteps = 23;
enum Pose { rest, side, cuddle, wave, pillow, game, stride, sniff, hop, burger, bite, sip, count };
using Key = std::tuple<Pose, Pose, int>;
struct Frame { int tile; double lift; double parcel; };
using cv::Mat;
using cv::Point;
using cv::Scalar;

double ease(double t, double a, double b) {
    const double p = std::clamp((t - a) / (b - a), 0., 1.);
    return p * p * (3 - 2 * p);
}

Mat cutout(const Mat& sheet, int cell) {
    const cv::Rect section((cell % 3) * 512, (cell / 3) * 512, 512, 512);
    Mat art = sheet(section).clone();
    Mat alpha;
    if (art.channels() == 4) {
        cv::extractChannel(art, alpha, 3);
        alpha.convertTo(alpha, CV_8U, 255. / 210, -45. * 255 / 210);
    } else {
        // The second render has an opaque backdrop. Seed the cutout inside the
        // forehead and coat, where the background cannot enter the foreground.
        const std::array<int, 6> centers{270,250,238,268,260,239};
        const std::array<cv::Rect, 6> bounds{{
            {106,2,328,500},{86,20,354,481},{39,2,395,472},
            {100,4,332,505},{83,4,353,505},{66,4,346,505}
        }};
        const int x = centers[cell];
        Mat labels(512,512,CV_8U,Scalar(cv::GC_BGD)), background, foreground;
        labels(bounds[cell]).setTo(cv::GC_PR_BGD);
        cv::ellipse(labels, {x,154}, {125,105}, 0, 0, 360, Scalar(cv::GC_PR_FGD), -1);
        cv::ellipse(labels, {x,154}, {80,72}, 0, 0, 360, Scalar(cv::GC_FGD), -1);
        cv::ellipse(labels, {x,343}, {62,83}, 0, 0, 360, Scalar(cv::GC_FGD), -1);
        cv::grabCut(art, labels, {}, background, foreground, 4, cv::GC_INIT_WITH_MASK);
        alpha = (labels & 1) * 255;
        cv::GaussianBlur(alpha, alpha, {3,3}, .6);
        cv::cvtColor(art, art, cv::COLOR_BGR2BGRA);
    }
    cv::insertChannel(alpha, art, 3);
    const auto body = cv::boundingRect(alpha > 170);
    const auto head = cv::boundingRect(alpha(cv::Rect(0,105,512,77)) > 170);
    if (body.height < 340 || head.width < 190) throw std::runtime_error("Pose cutout incomplete");
    const double scale = .232 * w / 108;
    const double x = w / 2. - scale * (head.x + head.width / 2.);
    const double y = h * 54.4 / 56. - scale * (body.y + body.height);
    Mat floating;
    art.convertTo(floating, CV_32FC4, 1. / 255);
    for (auto it = floating.begin<cv::Vec4f>(); it != floating.end<cv::Vec4f>(); ++it)
        for (int c = 0; c < 3; ++c) (*it)[c] *= (*it)[3];
    Mat fitted;
    cv::warpAffine(floating, fitted, cv::Matx23d(scale, 0, x, 0, scale, y),
                   {w,h}, cv::INTER_CUBIC);
    return fitted;
}

Mat gray(const Mat& art) {
    Mat out(h,w,CV_8U);
    for (int y=0; y<h; ++y)
        for (int x=0; x<w; ++x) {
            const auto p = art.at<cv::Vec4f>(y,x);
            out.at<unsigned char>(y,x) = cv::saturate_cast<unsigned char>(
                255 * (.114*p[0] + .587*p[1] + .299*p[2] + .86*(1-p[3])));
        }
    return out;
}

Mat warp(const Mat& art, const Mat& flow, double amount) {
    Mat grid(h,w,CV_32FC2), output, sampled;
    for (int y=0; y<h; ++y)
        for (int x=0; x<w; ++x)
            grid.at<cv::Vec2f>(y,x) = cv::Vec2f(x,y);
    Mat coordinates = grid - amount * flow;
    // Invert the forward field at each output pixel. A single subtraction
    // samples the field at the destination and smears moving details.
    for (int i=0; i<4; ++i) {
        cv::remap(flow, sampled, coordinates, {}, cv::INTER_LINEAR, cv::BORDER_REPLICATE);
        coordinates = .35 * coordinates + .65 * (grid - amount * sampled);
    }
    cv::remap(art, output, coordinates, {}, cv::INTER_CUBIC);
    return output;
}

struct Atlas {
    std::array<Mat,count> poses;
    std::map<std::pair<Pose,Pose>, Mat> flows;
    std::map<Key,int> tiles;
    std::vector<Mat> frames;

    explicit Atlas(const Mat& a, const Mat& b) {
        for (int i=0; i<6; ++i) {
            poses[i] = cutout(a,i);
            poses[i+6] = cutout(b,i);
        }
    }

    const Mat& flow(Pose from, Pose to) {
        const auto [it,created] = flows.try_emplace({from,to});
        if (created) {
            const auto solver = cv::DISOpticalFlow::create(cv::DISOpticalFlow::PRESET_MEDIUM);
            solver->setFinestScale(0);
            solver->setVariationalRefinementIterations(10);
            solver->calc(gray(poses[from]), gray(poses[to]), it->second);
        }
        return it->second;
    }

    int tile(Pose from, Pose to, double mix, bool mirror) {
        int phase = std::clamp(int(std::lround(transitionSteps * mix)),0,transitionSteps);
        if (from > to) { std::swap(from,to); phase = transitionSteps - phase; }
        if (phase == transitionSteps) { from = to; phase = 0; }
        if (!phase) to = from;
        if (from == rest && !phase) mirror = false;
        const Key key{from,to,phase};
        const auto [it,created] = tiles.try_emplace(key, static_cast<int>(frames.size()));
        const int id = mirror ? -1-it->second : it->second;
        if (!created) return id;

        Mat art = poses[from];
        if (from != to) {
            const double p = double(phase) / transitionSteps;
            const double weight = ease(p,.21,.79);
            art = warp(poses[from],flow(from,to),p) * (1-weight)
                + warp(poses[to],flow(to,from),1-p) * weight;
        }
        Mat soft;
        cv::GaussianBlur(art,soft,{3,3},.70);
        Mat tile(h,w,CV_8UC4,Scalar());
        for (int y=0; y<h; ++y)
            for (int x=0; x<w; ++x) {
                const auto p = art.at<cv::Vec4f>(y,x);
                if (p[3] < .012) continue;
                const auto low = soft.at<cv::Vec4f>(y,x);
                const double detail = (p[3] > .98 && low[3] > .98) ? .3 : 0.;
                auto& c = tile.at<cv::Vec4b>(y,x);
                for (int k=0; k<3; ++k)
                    c[k] = cv::saturate_cast<unsigned char>(255*(p[k]+detail*(p[k]-low[k]))/p[3]);
                c[3] = cv::saturate_cast<unsigned char>(255*p[3]);
            }
        frames.push_back(tile);
        return id;
    }

    Frame frame(Pose from, Pose to, double mix = 0, double lift = 0,
                bool mirror = false, double parcel = 60) {
        return {tile(from,to,mix,mirror), lift, parcel};
    }

    void writeSheet(const std::filesystem::path& path) const {
        const auto rows = (frames.size()+columns-1)/columns;
        Mat image(int(rows)*h,columns*w,CV_8UC4,Scalar());
        for (size_t i=0; i<frames.size(); ++i)
            frames[i].copyTo(image(cv::Rect(int(i%columns)*w,int(i/columns)*h,w,h)));
        const std::unique_ptr<avifImage,decltype(&avifImageDestroy)> picture(
            avifImageCreate(image.cols,image.rows,8,AVIF_PIXEL_FORMAT_YUV444), avifImageDestroy);
        const std::unique_ptr<avifEncoder,decltype(&avifEncoderDestroy)> encoder(
            avifEncoderCreate(), avifEncoderDestroy);
        if (!picture || !encoder) throw std::runtime_error("Cannot configure AVIF");
        picture->matrixCoefficients = AVIF_MATRIX_COEFFICIENTS_BT601;
        avifRGBImage pixels;
        avifRGBImageSetDefaults(&pixels,picture.get());
        pixels.format = AVIF_RGB_FORMAT_BGRA;
        pixels.pixels = image.data;
        pixels.rowBytes = image.step;
        if (avifImageRGBToYUV(picture.get(),&pixels) != AVIF_RESULT_OK)
            throw std::runtime_error("Cannot convert plush atlas to AVIF");

        encoder->quality = 66;
        encoder->qualityAlpha = 88;
        encoder->maxThreads = 4;
        encoder->speed = 6;
        encoder->tileRowsLog2 = 1;
        encoder->tileColsLog2 = 1;
        avifRWData data = AVIF_DATA_EMPTY;
        const bool encoded = avifEncoderWrite(encoder.get(),picture.get(),&data) == AVIF_RESULT_OK;
        std::ofstream out(path,std::ios::binary);
        if (encoded) out.write(reinterpret_cast<char*>(data.data),data.size);
        avifRWDataFree(&data);
        if (!encoded || !out) throw std::runtime_error("Cannot write plush atlas");
    }
};

void landscape(std::ostream& out) {
    const std::array<const char*,4> seasons{"spring","summer","autumn","winter"};
    const std::array<const char*,4> soil{"#d9e2c7","#b8cdaa","#e6d1b1","#d9e7ed"};
    const std::array<const char*,4> leaves{"#abc2aa","#8db59d","#c79c62","#bfd7e2"};
    const std::array<const char*,4> landmarks{
        R"SVG(<path d="M59 27Q58 23 59 19M59 24Q62 21 63 22" fill="none" stroke="#92ad83" stroke-width="1.1" stroke-linecap="round"/><circle cx="59" cy="17.4" r="2.8" fill="#e2a7b8"/><circle cx="59" cy="17.4" r="1.1" fill="#fff2db"/>)SVG",
        R"SVG(<path d="M59 17C51 10 53 22 59 19C65 22 67 10 59 17Z" fill="#ebc583"/><path d="M59 16v4" stroke="#ad8c6d" stroke-linecap="round"/>)SVG",
        R"SVG(<path d="M58 21Q56 18 53 16L56 15L56 12L59 14L61 11L62 15L65 15Q64 19 58 21Z" fill="#cb9269"/><path d="M57 22L61 15" stroke="#b47c59" stroke-width=".6"/>)SVG",
        R"SVG(<path d="M59 13v9m-4-7 8 5m0-5-8 5" stroke="#a8c7db" stroke-width="1.4" stroke-linecap="round"/><circle cx="59" cy="17.5" r="1.2" fill="#fff"/>)SVG"
    };
    out << R"SVG(<svg class="pb-landscape" viewBox="0 0 120 34" preserveAspectRatio="none" opacity="0" aria-hidden="true">
<defs><radialGradient id="plush-trail"><stop stop-color="white"/><stop offset="1" stop-color="#b9a7a0" stop-opacity="0"/></radialGradient></defs>)SVG";
    for (int i=0;i<4;++i) {
        out << "<g class=\"pb-season pb-season-" << seasons[i] << "\">";
        out << "<ellipse cx=\"60\" cy=\"28\" rx=\"59\" ry=\"4.6\" fill=\"" << soil[i] << "\" opacity=\".72\"/>";
        out << R"SVG(<ellipse cx="60" cy="26.4" rx="58" ry="5.2" fill="url(#plush-trail)" opacity=".55"/>)SVG";
        out << "<g fill=\"" << leaves[i] << "\" opacity=\".6\"><ellipse cx=\"11\" cy=\"26\" rx=\"5\" ry=\"1.9\"/><ellipse cx=\"107\" cy=\"26\" rx=\"6\" ry=\"2.1\"/></g>";
        out << R"SVG(<g class="pb-glints" fill="#fff"><circle cx="16" cy="18" r=".7"/><circle cx="91" cy="13" r=".55"/><circle cx="101" cy="21" r=".6"/></g>
<g class="pb-landmark"><g class="pb-discovery">)SVG";
        out << landmarks[i] << "</g></g></g>\n";
    }
    out << "</svg>\n";
}

struct Clip { const char* name; int ms; int frames; bool repeat; };

Frame motion(Atlas& a, const std::string& name, double t) {
    if (name == "walk") {
        const double phase = std::fmod(t,.32);
        const double step = ease(phase,0,.13) - ease(phase,.15,.32);
        return a.frame(side,stride,(t < .32 ? .98 : .72)*step,-.65*step);
    }
    if (name == "stepOff" || name == "stepOffLeft" || name == "settle" || name == "settleLeft") {
        const double p = ease(t,0,.205);
        return a.frame(rest,side,name.find("stepOff")==0 ? p : 1-p,0,name.find("Left")!=std::string::npos);
    }
    if (name == "turn" || name == "turnLeft")
        return a.frame(rest,side,t<=.22 ? 1-ease(t,0,.22) : ease(t,.22,.414),
                       -.52*(ease(t,0,.20)-ease(t,.22,.414)),(name=="turnLeft")!=(t>.22));
    if (name == "inspect" || name == "inspectUp") {
        const double p = ease(t,.24,1.02)-ease(t,1.77,2.59);
        return a.frame(side,sniff,p,name=="inspectUp" ? -.9*p : 0);
    }
    if (name == "pat") {
        const double p = ease(t,.04,.44)-ease(t,1.86,2.33);
        return a.frame(rest,cuddle,p,p*.65*std::sin(t*5));
    }
    if (name == "dance") {
        if (t < 1.1) return a.frame(rest,cuddle,ease(t,.08,.57),-.65*(ease(t,.45,.8)-ease(t,.84,1.1)));
        if (t >= 2.05) return a.frame(cuddle,rest,ease(t,2.13,3.05));
        const double jump = ease(t,1.1,1.43)-ease(t,1.61,1.91);
        return a.frame(cuddle,hop,jump,-3.55*jump);
    }
    if (name == "feed") {
        if (t < 1.43) return a.frame(rest,burger,ease(t,.24,1.31));
        if (t < 3.79) {
            const double p = ease(t,1.43,1.99)-ease(t,2.13,2.64)+ease(t,3.04,3.62);
            const double chew = ease(t,2.17,2.32)-ease(t,3.61,3.79);
            return a.frame(burger,bite,p,.30*chew*std::sin(t*25));
        }
        if (t < 4.49) return a.frame(bite,rest,ease(t,3.79,4.42));
        if (t < 7.83) return a.frame(rest,sip,ease(t,4.59,5.86)-ease(t,6.72,7.76));
        const double p = ease(t,7.96,8.52)-ease(t,9.2,9.85);
        return a.frame(rest,cuddle,p,-1.3*(ease(t,8.74,8.91)-ease(t,8.91,9.17)));
    }
    if (name == "sleep" || name == "game") {
        const bool sleeping = name == "sleep";
        const double p = ease(t,.21,1.17)-ease(t,sleeping ? 8.94 : 7.08,sleeping ? 9.69 : 7.84);
        return a.frame(rest,sleeping ? pillow : game,p,.48*p*std::sin(t*(sleeping ? 2.9 : 5.7)));
    }
    if (name == "drag")
        return a.frame(rest,hop,.6,-1.8-.8*std::sin(t/.72*2*std::acos(-1.0)));
    if (name == "enter") {
        const double rise = 58 - 61*ease(t,.23,1.19) + 3*ease(t,1.19,1.41);
        const double parcel = 38+22*ease(t,.83,1.44);
        return a.frame(rest,wave,ease(t,1.4,1.63)-ease(t,1.91,2.14),rise,false,parcel);
    }
    if (name == "exit") {
        const double waveAmount = ease(t,.04,.23)-ease(t,.50,.78);
        const double parcel = 60-22*ease(t,.78,1.14);
        return a.frame(rest,wave,waveAmount,58*ease(t,.84,1.43),false,parcel);
    }
    const double p = .67*(ease(t,1.25,2.13)-ease(t,3.52,4.51));
    return a.frame(rest,cuddle,p,.25*std::sin(t/5.6*4*std::acos(-1.0)));
}

void write(Atlas& atlas, std::ostream& out) {
    const Clip clips[]{
        {"rest",5600,180,true},{"walk",640,28,true},{"stepOff",240,9,false},{"stepOffLeft",240,9,false},
        {"turn",440,18,false},{"turnLeft",440,18,false},{"settle",240,9,false},{"settleLeft",240,9,false},
        {"inspect",2700,97,false},{"inspectUp",2700,97,false},{"pat",2400,86,false},{"feed",10000,360,false},
        {"dance",3200,115,false},{"sleep",10000,360,false},{"game",8000,288,false},
        {"enter",2200,79,false},{"exit",1700,61,false},{"drag",720,26,true}
    };
    out << R"SVG(<svg xmlns="http://www.w3.org/2000/svg" class="pb-rig" width="48" height="56" viewBox="0 0 48 56" overflow="visible" aria-hidden="true">
<title>小熊熊 · 奶油绒毛</title><desc>戴粉色眼镜、穿盖膝白大褂的奶油色绒毛小熊。</desc>
<metadata class="pb-motion">{"columns":)SVG" << columns << ",\"stride\":4,\"clips\":{";
    bool first = true;
    for (const auto& clip : clips) {
        if (!first) out << ',';
        first = false;
        out << std::quoted(clip.name) << ":[" << clip.ms << ',' << std::boolalpha << clip.repeat << ",[";
        std::vector<Frame> poses;
        for (int i=0; i<clip.frames; ++i) {
            if (i) out << ',';
            const double duration = clip.repeat ? clip.frames : clip.frames-1;
            poses.push_back(motion(atlas,clip.name,i*clip.ms/(1000.*duration)));
            out << poses.back().tile;
        }
        out << "],[";
        out << std::fixed << std::setprecision(2);
        for (size_t i=0; i<poses.size(); ++i) {
            if (i) out << ',';
            out << poses[i].lift;
        }
        out << "],[";
        for (size_t i=0; i<poses.size(); ++i) {
            if (i) out << ',';
            out << poses[i].parcel;
        }
        out << "]]";
    }
    out << R"SVG(}}</metadata>
<defs><linearGradient id="plush-box" x2="0%" y2="100%"><stop stop-color="#e5c49e"/><stop offset="1" stop-color="#be936b"/></linearGradient><radialGradient id="plush-shadow"><stop stop-color="#8a6f60" stop-opacity=".24"/><stop offset="1" stop-color="#8a6f60" stop-opacity="0"/></radialGradient></defs>
<ellipse class="pb-contact" cx="24" cy="54.4" rx="19" ry="2.8" fill="url(#plush-shadow)"/>
<svg class="pb-window" width="48" height="56" viewBox="0 0 48 56" overflow="hidden"><g class="pb-body"><image class="pb-atlas" width=")SVG"
        << columns*48 << "\" height=\"" << (atlas.frames.size()+columns-1)/columns*56
        << R"SVG(" href="pet-plush-hd.avif?v=1"/></g>
<g class="pb-parcel" style="transform:translateY(60px)">
<path d="M19 3C12-4 19-5 24 1C30-5 36-4 29 3Z" fill="#c78992"/>
<rect x="11" y="3" width="26" height="15" rx="2" fill="url(#plush-box)"/>
<rect x="10" y="2.4" width="28" height="3.6" rx="1.2" fill="#e9c9a3"/>
<path d="M22 2.5h4V18h-4Z" fill="#f3d6ad" opacity=".9"/>
</g></svg>
)SVG";
    landscape(out);
    out << "</svg>\n";
}
}  // namespace

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: build-plush poses.png actions.png output.svg\n";
        return 1;
    }
    try {
        cv::setNumThreads(4);
        const Mat poses = cv::imread(argv[1],cv::IMREAD_UNCHANGED);
        const Mat actions = cv::imread(argv[2],cv::IMREAD_UNCHANGED);
        if (poses.size() != cv::Size(1536,1024) || actions.size() != poses.size())
            throw std::runtime_error("Expected two 1536 by 1024 pose sheets");
        Atlas atlas(poses,actions);
        std::ofstream out(argv[3]);
        write(atlas,out);
        if (!out) throw std::runtime_error("Cannot write plush rig");
        const auto folder = std::filesystem::path(argv[3]).parent_path();
        atlas.writeSheet(folder / "pet-plush-hd.avif");
        const auto still = folder / "pet-plush-hd-still.webp";
        if (!cv::imwrite(still.string(),atlas.frames.front(),{cv::IMWRITE_WEBP_QUALITY,85}))
            throw std::runtime_error("Cannot write plush still");
        std::cout << atlas.frames.size() << " plush frames\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
