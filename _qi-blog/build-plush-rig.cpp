// g++ -std=c++17 -O2 _qi-blog/build-plush-rig.cpp $(pkg-config --cflags --libs opencv4) -o /tmp/build-plush-rig
// /tmp/build-plush-rig poses.png actions.png assets
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using cv::Mat;
using cv::Point;
using cv::Scalar;
using Json = nlohmann::json;

namespace {
Mat polygon(std::initializer_list<Point> points) {
    Mat m(512,512,CV_8U,Scalar());
    cv::fillPoly(m,std::vector<std::vector<Point>>{points},Scalar(255),cv::LINE_AA);
    return m;
}
Mat cell(const Mat& sheet,int n) {
    return sheet(cv::Rect(n%3*512,n/3*512,512,512)).clone();
}
struct Pack {
    Mat image{1536,1024,CV_8UC4,Scalar()};
    Json parts=Json::object();
    int x=2,y=2,row=0;
    void add(const std::string& name,const Mat& source,const Mat& mask) {
        const auto box=cv::boundingRect(mask);
        if(x+box.width+2>image.cols){x=2;y+=row+4;row=0;}
        if(y+box.height+2>image.rows)throw std::runtime_error("Atlas full");
        Mat out=source(box).clone();
        if(out.channels()==3)cv::cvtColor(out,out,cv::COLOR_BGR2BGRA);
        for(int j=0;j<out.rows;++j)for(int i=0;i<out.cols;++i) {
            auto& p=out.at<cv::Vec4b>(j,i);
            p[3]=cv::saturate_cast<uchar>(p[3]*mask.at<uchar>(box.y+j,box.x+i)/255.);
        }
        out.copyTo(image(cv::Rect(x,y,box.width,box.height)));
        parts[name]={{"rect",{x,y,box.width,box.height}},{"origin",{box.x,box.y}}};
        x+=box.width+4;row=std::max(row,box.height);
    }
};
}

int main(int argc,char**argv) {
    if(argc!=4){std::cerr<<"Usage: build-plush-rig poses.png actions.png output-directory\n";return 1;}
    try {
        const Mat poses=cv::imread(argv[1],cv::IMREAD_UNCHANGED);
        const Mat actions=cv::imread(argv[2],cv::IMREAD_UNCHANGED);
        if(poses.size()!=cv::Size(1536,1024)||actions.size()!=poses.size())throw std::runtime_error("Invalid sheets");
        Mat rest=cell(poses,0),alpha;
        cv::extractChannel(rest,alpha,3);
        alpha.convertTo(alpha,CV_8U,255./210,-45.*255/210);
        cv::insertChannel(alpha,rest,3);
        Pack pack;

        // One drawing supplies every body part. Shared texture and overlapping
        // shoulder/hip margins keep the coat attached during deformation.
        pack.add("coat",rest,polygon({{211,254},{357,254},{366,324},{372,375},{390,445},{169,449},{198,375},{205,324}}));
        for(const auto& [name,cx]:std::vector<std::pair<std::string,int>>{{"leftArm",208},{"rightArm",366}}) {
            Mat arm(512,512,CV_8UC4,Scalar()),mask(512,512,CV_8U,Scalar());
            for(int y=247;y<415;++y)for(int x=cx-36;x<=cx+36;++x) {
                const double dx=x-cx,dy=y-386;
                const double paw=std::clamp(25.4-std::hypot(dx,dy),0.,1.);
                const double radius=31-5*std::clamp((y-282)/85.,0.,1.);
                const double edge=std::min(radius-std::abs(dx),y<278 ? 31-std::hypot(dx,double(y-278)) : 370.-y);
                const double sleeve=std::clamp(edge+.5,0.,1.);
                if(paw<=0&&sleeve<=0)continue;
                const bool fabric=sleeve>0;
                const int sx=fabric ? cv::borderInterpolate(int(394+dx*.45),512,cv::BORDER_REFLECT) : int(403+dx*.82);
                const int sy=fabric ? int(314+std::clamp((y-278)*.19,-7.,17.)) : int(375+dy*.82);
                auto pixel=rest.at<cv::Vec4b>(sy,sx);
                const double shade=fabric ? 1.015-.09*std::pow(dx/radius,2) : 1.;
                for(int c=0;c<3;++c)pixel[c]=cv::saturate_cast<uchar>(pixel[c]*shade);
                pixel[3]=cv::saturate_cast<uchar>(255*std::max(paw,sleeve));
                arm.at<cv::Vec4b>(y,x)=pixel;mask.at<uchar>(y,x)=255;
            }
            pack.add(name,arm,mask);
        }
        pack.add("leftFoot",rest,polygon({{193,427},{283,427},{283,506},{194,506}}));
        pack.add("rightFoot",rest,polygon({{280,427},{374,427},{375,507},{281,507}}));

        Mat head=rest.clone(),remove(512,512,CV_8U,Scalar()),eyeL=remove.clone(),eyeR=remove.clone(),mouth=remove.clone();
        cv::ellipse(eyeL,{245,157},{16,24},0,0,360,Scalar(255),-1,cv::LINE_AA);
        cv::ellipse(eyeR,{329,156},{16,24},0,0,360,Scalar(255),-1,cv::LINE_AA);
        cv::ellipse(mouth,{288,197},{29,15},0,0,360,Scalar(255),-1,cv::LINE_AA);
        // Keep only the dark facial marks; glasses, nose and fur never change.
        for(int j=0;j<512;++j)for(int i=0;i<512;++i) {
            auto p=rest.at<cv::Vec4b>(j,i);
            const bool dark=p[0]<105&&p[1]<130&&p[2]<160;
            if(!dark||j<190) mouth.at<uchar>(j,i)=0;
        }
        remove=eyeL|eyeR|mouth;
        cv::dilate(remove,remove,cv::getStructuringElement(cv::MORPH_ELLIPSE,{5,5}));
        Mat rgb,healed;
        cv::cvtColor(head,rgb,cv::COLOR_BGRA2BGR);
        cv::inpaint(rgb,remove,healed,5,cv::INPAINT_TELEA);
        cv::cvtColor(healed,head,cv::COLOR_BGR2BGRA);
        cv::insertChannel(alpha,head,3);
        pack.add("head",head,polygon({{80,0},{460,0},{460,250},{396,263},{288,265},{168,263},{80,242}}));
        pack.add("leftEye",rest,eyeL);pack.add("rightEye",rest,eyeR);pack.add("mouth",rest,mouth);
        Mat happy(512,512,CV_8UC4,Scalar());
        cv::ellipse(happy,{245,158},{13,7},0,190,350,Scalar(38,51,73,255),4,cv::LINE_AA);
        pack.add("happyEye",happy,polygon({{226,145},{264,145},{264,168},{226,168}}));

        auto prop=[&](std::string name,const Mat& sheet,int n,std::initializer_list<Point> points){
            pack.add(name,cell(sheet,n),polygon(points));
        };
        Mat burgerMask=polygon({{244,257},{251,248},{266,246},{288,247},{304,254},{315,265},{308,320},{252,321},{242,268}});
        pack.add("burger",cell(actions,3),burgerMask);
        cv::circle(burgerMask,{298,251},12,Scalar(),-1,cv::LINE_AA);
        pack.add("burgerBitten",cell(actions,3),burgerMask);
        prop("cup",actions,5,{{204,248},{232,245},{239,207},{247,207},{241,246},{268,247},{275,252},{272,263},{267,326},{215,326},{209,263}});
        prop("controller",poses,5,{{187,304},{200,289},{218,288},{238,296},{258,293},{281,292},{297,298},{311,317},{308,343},{291,346},{276,332},{222,331},{207,345},{189,338}});
        prop("pillow",poses,4,{{172,244},{187,234},{321,222},{345,217},{350,226},{340,276},{323,289},{190,288},{172,278},{162,260}});
        Mat stage(512,512,CV_8UC4,Scalar());
        cv::ellipse(stage,{288,501},{134,12},0,0,360,Scalar(98,116,135,38),-1,cv::LINE_AA);
        pack.add("shadow",stage,polygon({{148,486},{428,486},{428,512},{148,512}}));
        stage.setTo(Scalar());
        cv::line(stage,{355,369},{350,498},Scalar(168,193,209,255),8,cv::LINE_AA);
        cv::line(stage,{431,369},{440,498},Scalar(168,193,209,255),8,cv::LINE_AA);
        cv::ellipse(stage,{392,367},{64,9},0,0,360,Scalar(183,207,223,255),-1,cv::LINE_AA);
        cv::ellipse(stage,{392,363},{64,8},0,0,360,Scalar(223,238,245,255),-1,cv::LINE_AA);
        pack.add("table",stage,polygon({{325,351},{460,351},{460,505},{325,505}}));
        stage.setTo(Scalar());
        cv::rectangle(stage,{171,403},{405,500},Scalar(153,191,222,255),-1);
        cv::rectangle(stage,{166,391},{410,414},Scalar(180,211,235,255),-1);
        cv::rectangle(stage,{276,391},{299,500},Scalar(183,166,219,255),-1);
        cv::ellipse(stage,{268,386},{22,10},23,0,360,Scalar(168,143,201,255),5,cv::LINE_AA);
        cv::ellipse(stage,{307,386},{22,10},-23,0,360,Scalar(168,143,201,255),5,cv::LINE_AA);
        pack.add("parcel",stage,polygon({{160,370},{416,370},{416,506},{160,506}}));
        const auto output=std::filesystem::path(argv[3]);
        pack.image=pack.image(cv::Rect(0,0,1024,pack.y+pack.row+2));
        if(!cv::imwrite((output/"pet-plush-parts.webp").string(),pack.image,{cv::IMWRITE_WEBP_QUALITY,95}))throw std::runtime_error("Cannot save texture");
        const Json rig={{"texture","pet-plush-parts.webp?v=1"},{"size",{pack.image.cols,pack.image.rows}},{"parts",pack.parts}};
        std::ofstream out(output/"pet-plush-parts.json");out<<rig.dump();
        if(!out)throw std::runtime_error("Cannot save rig");
        std::ifstream original(output/"pet-plush-hd.svg");
        const std::string old((std::istreambuf_iterator<char>(original)),{});
        const auto start=old.find("<svg class=\"pb-landscape\"");
        const auto end=old.find("</svg>",start);
        if(start==std::string::npos||end==std::string::npos)throw std::runtime_error("Missing existing seasonal scene");
        std::string scene=old.substr(start,end+6-start);
        scene.insert(4," xmlns=\"http://www.w3.org/2000/svg\"");
        std::ofstream(output/"pet-plush-scene.svg")<<scene;
        cv::imwrite("/tmp/qi-plush-parts-debug.png",pack.image);
        std::cout<<pack.parts.size()<<" parts, "<<pack.image.cols<<" x "<<pack.image.rows<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
