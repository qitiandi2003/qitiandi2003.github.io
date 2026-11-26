// g++ -std=c++17 -O2 _qi-blog/build-plush.cpp $(pkg-config --cflags --libs opencv4 libwebp) -o /tmp/build-plush
// /tmp/build-plush poses.png actions.png assets/pet-plush-rig.svg
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <webp/encode.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {
constexpr int w = 108, h = 126, columns = 16, transitionSteps = 5;
enum Pose { rest, side, cuddle, wave, pillow, game, stride, sniff, hop, burger, bite, sip, count };
using Key = std::tuple<Pose, Pose, int, int, bool, int>;
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
    const double scale = .232;
    const double x = w / 2. - scale * (head.x + head.width / 2.);
    const double y = h - 4. - scale * (body.y + body.height);
    Mat floating;
    art.convertTo(floating, CV_32FC4, 1. / 255);
    for (auto it = floating.begin<cv::Vec4f>(); it != floating.end<cv::Vec4f>(); ++it)
        for (int c = 0; c < 3; ++c) (*it)[c] *= (*it)[3];
    Mat fitted;
    cv::warpAffine(floating, fitted, cv::Matx23d(scale, 0, x, 0, scale, y),
                   {w,h}, cv::INTER_LINEAR);
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
    Mat coordinates(h,w,CV_32FC2), output;
    for (int y=0; y<h; ++y)
        for (int x=0; x<w; ++x)
            coordinates.at<cv::Vec2f>(y,x) = cv::Vec2f(x,y) - amount * flow.at<cv::Vec2f>(y,x);
    cv::remap(art, output, coordinates, {}, cv::INTER_LINEAR);
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
        if (created)
            cv::DISOpticalFlow::create(cv::DISOpticalFlow::PRESET_MEDIUM)->calc(
                gray(poses[from]), gray(poses[to]), it->second);
        return it->second;
    }

    int tile(Pose from, Pose to, double mix = 0, int lift = 0, bool mirror = false, int box = 0) {
        int phase = std::clamp(int(std::lround(transitionSteps * mix)),0,transitionSteps);
        if (phase == transitionSteps) { from = to; phase = 0; }
        if (!phase) to = from;
        if (from == rest && !phase) mirror = false;
        const Key key{from,to,phase,lift,mirror,box};
        const auto [it,created] = tiles.try_emplace(key, static_cast<int>(frames.size()));
        if (!created) return it->second;

        Mat art = poses[from];
        if (from != to) {
            const double p = double(phase) / transitionSteps;
            art = warp(poses[from],flow(from,to),p) * (1-p)
                + warp(poses[to],flow(to,from),1-p) * p;
        }
        if (lift || mirror) {
            Mat moved;
            cv::warpAffine(art,moved,cv::Matx23d(mirror ? -1 : 1,0,mirror ? w-1 : 0,0,1,lift),
                           {w,h},cv::INTER_LINEAR);
            art = moved;
        }
        Mat tile(h,w,CV_8UC4,Scalar());
        for (int y=0; y<h; ++y)
            for (int x=0; x<w; ++x) {
                const auto p = art.at<cv::Vec4f>(y,x);
                if (p[3] < .012) continue;
                auto& c = tile.at<cv::Vec4b>(y,x);
                for (int k=0; k<3; ++k) c[k] = cv::saturate_cast<unsigned char>(255*p[k]/p[3]);
                c[3] = cv::saturate_cast<unsigned char>(255*p[3]);
            }
        if (box) {
            const int y = 94 + (box > 1 ? box - 2 : 0);
            cv::rectangle(tile, {27,y}, {81,h-1}, {104,142,177,255}, -1);
            cv::rectangle(tile, {30,y+4}, {78,h-1}, {137,175,207,255}, -1);
            cv::rectangle(tile, {50,y}, {58,h-1}, {176,202,229,255}, -1);
            cv::rectangle(tile, {24,y}, {84,y+4}, {142,180,213,255}, -1);
            if (box == 1) {
                cv::ellipse(tile,{49,y-3},{8,4},25,0,360,{147,140,204,255},-1,cv::LINE_AA);
                cv::ellipse(tile,{59,y-3},{8,4},-25,0,360,{147,140,204,255},-1,cv::LINE_AA);
            }
        }
        frames.push_back(tile);
        return it->second;
    }

    std::string webp() const {
        const auto rows = (frames.size()+columns-1)/columns;
        Mat image(int(rows)*h,columns*w,CV_8UC4,Scalar());
        for (size_t i=0; i<frames.size(); ++i)
            frames[i].copyTo(image(cv::Rect(int(i%columns)*w,int(i/columns)*h,w,h)));
        uint8_t* data = nullptr;
        const auto size = WebPEncodeBGRA(image.data,image.cols,image.rows,int(image.step),79,&data);
        if (!size) throw std::runtime_error("Cannot encode plush atlas");
        constexpr char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string encoded;
        encoded.reserve((size+2)/3*4);
        for (size_t i=0; i<size; i+=3) {
            const unsigned n = data[i]<<16 | (i+1<size ? data[i+1]<<8 : 0) | (i+2<size ? data[i+2] : 0);
            encoded += chars[n>>18];
            encoded += chars[(n>>12)&63];
            encoded += i+1<size ? chars[(n>>6)&63] : '=';
            encoded += i+2<size ? chars[n&63] : '=';
        }
        WebPFree(data);
        return encoded;
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

int motion(Atlas& a, const std::string& name, double t) {
    auto n = [](double x) { return int(std::lround(x)); };
    if (name == "walk") {
        const double step = std::pow(std::sin(t/.64*2*std::acos(-1.0)),2);
        return a.tile(side,stride,.8*step,-n(2*step));
    }
    if (name == "stepOff" || name == "stepOffLeft" || name == "settle" || name == "settleLeft") {
        const double p = ease(t,0,.2);
        return a.tile(rest,side,name.find("stepOff")==0 ? p : 1-p,0,name.find("Left")!=std::string::npos);
    }
    if (name == "turn" || name == "turnLeft")
        return a.tile(rest,side,t<=.2 ? 1-ease(t,0,.2) : ease(t,.2,.40),0,(name=="turnLeft")!=(t>.2));
    if (name == "inspect" || name == "inspectUp")
        return a.tile(side,sniff,ease(t,.24,1.02)-ease(t,1.77,2.59),name=="inspectUp" ? -n(2*(ease(t,.25,1)-ease(t,1.71,2.6))) : 0);
    if (name == "pat") {
        const double p = ease(t,.04,.44)-ease(t,1.86,2.33);
        return a.tile(rest,cuddle,p,n(p*std::sin(t*5)));
    }
    if (name == "dance") {
        if (t < 1.10) return a.tile(rest,cuddle,ease(t,.08,.57),-n(2*ease(t,.45,.9)));
        if (t >= 2.05) return a.tile(cuddle,rest,ease(t,2.13,3.05));
        const double jump = ease(t,1.10,1.43)-ease(t,1.61,1.91);
        return a.tile(cuddle,hop,jump,-n(8*jump));
    }
    if (name == "feed") {
        if (t < 1.43) return a.tile(rest,burger,ease(t,.24,1.31));
        if (t < 3.79) {
            const double p = ease(t,1.43,1.99)-ease(t,2.13,2.64)+ease(t,3.04,3.62);
            return a.tile(burger,bite,p,n(ease(t,2.18,2.55)*std::sin(t*27)));
        }
        if (t < 4.49) return a.tile(bite,rest,ease(t,3.80,4.42));
        if (t < 7.83) return a.tile(rest,sip,ease(t,4.59,5.86)-ease(t,6.72,7.76));
        return a.tile(rest,cuddle,ease(t,7.96,8.52)-ease(t,9.2,9.85),-n(3*(ease(t,8.74,8.91)-ease(t,8.91,9.17))));
    }
    if (name == "sleep" || name == "game") {
        const bool sleeping = name == "sleep";
        const double p = ease(t,.21,1.17)-ease(t,sleeping ? 8.94 : 7.08,sleeping ? 9.69 : 7.84);
        return a.tile(rest,sleeping ? pillow : game,p,n(p*std::sin(t*(sleeping ? 2.9 : 5.7))));
    }
    if (name == "drag") return a.tile(rest,hop,.6,-4-n(2*std::sin(t/.72*2*std::acos(-1.0))));
    if (name == "enter") {
        if (t < .23) return a.tile(rest,rest,0,h,false,1);
        if (t < 1.44) return a.tile(rest,wave,ease(t,1.21,1.48),n(64-70*ease(t,.26,1.19)+6*ease(t,1.2,1.4)),
                                    false,2+n(36*ease(t,.83,1.4)));
        return a.tile(rest,wave,ease(t,1.4,1.63)-ease(t,1.91,2.14));
    }
    if (name == "exit") {
        if (t < .8) return a.tile(rest,wave,ease(t,.04,.23)-ease(t,.50,.78));
        return a.tile(rest,rest,0,n(132*ease(t,.84,1.43)),false,t>1.42 ? 1 : 2+n(35*(1-ease(t,.8,1.13))));
    }
    return a.tile(rest,cuddle,.67*(ease(t,1.25,2.13)-ease(t,3.52,4.51)),0);
}

void write(Atlas& atlas, std::ostream& out) {
    const Clip clips[]{
        {"rest",5600,28,true},{"walk",640,16,true},{"stepOff",240,6,false},{"stepOffLeft",240,6,false},
        {"turn",440,11,false},{"turnLeft",440,11,false},{"settle",240,6,false},{"settleLeft",240,6,false},
        {"inspect",2700,36,false},{"inspectUp",2700,36,false},{"pat",2400,40,false},{"feed",10000,160,false},
        {"dance",3200,48,false},{"sleep",10000,80,false},{"game",8000,96,false},
        {"enter",2200,44,false},{"exit",1700,34,false},{"drag",720,12,true}
    };
    out << R"SVG(<svg xmlns="http://www.w3.org/2000/svg" class="pb-rig" width="48" height="56" viewBox="0 0 48 56" overflow="visible" aria-hidden="true">
<title>小熊熊 · 奶油绒毛</title><desc>戴粉色眼镜、穿盖膝白大褂的奶油色绒毛小熊。</desc>
<metadata class="pb-motion">{"columns":)SVG" << columns << ",\"stride\":4,\"clips\":{";
    bool first = true;
    for (const auto& clip : clips) {
        if (!first) out << ',';
        first = false;
        out << std::quoted(clip.name) << ":[" << clip.ms << ',' << std::boolalpha << clip.repeat << ",[";
        for (int i=0; i<clip.frames; ++i) {
            if (i) out << ',';
            out << motion(atlas,clip.name,i*clip.ms/(1000.*clip.frames));
        }
        out << "]]";
    }
    out << R"SVG(}}</metadata>
<defs><radialGradient id="plush-shadow"><stop stop-color="#8a6f60" stop-opacity=".24"/><stop offset="1" stop-color="#8a6f60" stop-opacity="0"/></radialGradient></defs>
<ellipse class="pb-contact" cx="24" cy="54.4" rx="19" ry="2.8" fill="url(#plush-shadow)"/>
<svg class="pb-window" width="48" height="56" viewBox="0 0 48 56" overflow="hidden"><image class="pb-atlas" width=")SVG"
        << columns*48 << "\" height=\"" << (atlas.frames.size()+columns-1)/columns*56
        << "\" href=\"data:image/webp;base64," << atlas.webp() << "\"/></svg>\n";
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
        const auto still = std::filesystem::path(argv[3]).parent_path() / "pet-plush-still.webp";
        if (!cv::imwrite(still.string(),atlas.frames.front(),{cv::IMWRITE_WEBP_QUALITY,85}))
            throw std::runtime_error("Cannot write plush still");
        std::cout << atlas.frames.size() << " plush frames\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
