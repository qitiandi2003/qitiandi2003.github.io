// Rebuild from the repository root:
// g++ -std=c++17 -O2 _qi-blog/build-pet.cpp -lpng -o /tmp/build-pet && /tmp/build-pet
#include <png.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using Point = std::pair<int, int>;
using Row = std::pair<int, int>;  // Inclusive x coordinates of a silhouette on one scanline.
constexpr int width = 48;
constexpr int height = 56;

// Golden plush fur, cream fabric and strawberry-rose details share one palette.
const std::map<char, std::string> palette = {
    {'o', "#895e43"}, {'s', "#b18052"}, {'t', "#ca995f"}, {'g', "#dca66e"},
    {'f', "#eabd86"}, {'h', "#f3cc97"}, {'l', "#f9daab"}, {'e', "#ae5341"},
    {'p', "#d98b73"}, {'r', "#eea779"}, {'m', "#ecc58e"}, {'w', "#fff3df"},
    {'n', "#724632"}, {'k', "#563826"}, {'O', "#8e8171"}, {'S', "#aaa99a"},
    {'M', "#becac2"}, {'W', "#e8ede4"}, {'F', "#fff8e8"}, {'B', "#709397"},
    {'A', "#83a8a8"}, {'C', "#91bab5"}, {'T', "#66898e"}, {'D', "#8aa6ad"},
    {'I', "#dce5d5"}, {'z', "#69839b"}, {'v', "#818aab"}, {'a', "#aab4cd"},
    {'b', "#d6dbea"}, {'d', "#edeef2"}, {'y', "#e2a052"}, {'j', "#f8c272"},
    {'u', "#925832"}, {'q', "#593b36"}, {'H', "#70894e"}, {'G', "#acc078"},
    {'R', "#bb5949"}, {'X', "#ab5b50"}, {'V', "#d46e61"}, {'P', "#ed9d82"},
    {'E', "#354b59"}, {'L', "#7ba59f"}, {'Y', "#8fc29e"}, {'Z', "#dad995"},
    {'J', "#cd7889"}, {'K', "#ffd4d1"}, {'N', "#a74e67"}, {'Q', "#ee9b9f"},
    {'0', "#795e4a"}, {'1', "#a77e57"}, {'2', "#c39c70"}, {'3', "#dab78b"},
    {'4', "#ead0a8"}, {'5', "#f4e1bd"}, {'6', "#8da6a2"},
    {'c', "#526c82"}, {'7', "#43596d"},
};

template<int Width, int Height>
class PixelGrid {
public:
    void dot(int x, int y, char ink) { pixels.at(y).at(x) = ink; }
    char at(int x, int y) const { return pixels[y][x]; }

    void over(const PixelGrid& source, int dx = 0, int dy = 0) {
        for (int y = std::max(0, -dy); y < std::min(Height, Height - dy); ++y)
            for (int x = std::max(0, -dx); x < std::min(Width, Width - dx); ++x)
                if (const char ink = source.at(x, y)) dot(x + dx, y + dy, ink);
    }

    void span(int y, int first, int last, char ink) {
        for (int x = first; x <= last; ++x) dot(x, y, ink);
    }

    void stamp(int x, int y, std::initializer_list<std::string> rows) {
        for (const auto& row : rows) {
            int column = x;
            for (char ink : row) {
                if (ink != '.') dot(column, y, ink);
                ++column;
            }
            ++y;
        }
    }

    void dots(std::initializer_list<Point> points, char ink) {
        for (auto [x, y] : points) dot(x, y, ink);
    }

    template<class Paint>
    void silhouette(int top, const std::vector<Row>& rows, char outline, Paint shade) {
        auto inside = [&](int x, int y) {
            const int index = y - top;
            return index >= 0 && index < static_cast<int>(rows.size()) &&
                   x >= rows[index].first && x <= rows[index].second;
        };
        for (int index = 0; index < static_cast<int>(rows.size()); ++index) {
            const int y = top + index;
            const auto [left, right] = rows[index];
            for (int x = left; x <= right; ++x) {
                bool edge = !inside(x - 1, y) || !inside(x + 1, y) ||
                            !inside(x, y - 1) || !inside(x, y + 1);
                dot(x, y, edge ? outline : shade(x, y, left, right));
            }
        }
    }

    void write(std::ostream& out, const std::map<char, std::string>& colors = palette) const {
        // The seasonal scenery is SVG so the path can stretch with the walking range.
        std::map<char, std::ostringstream> paths;
        for (int y = 0; y < Height; ++y) {
            for (int x = 0; x < Width;) {
                const int start = x;
                const char ink = pixels[y][x++];
                while (x < Width && pixels[y][x] == ink) ++x;
                if (ink) paths[ink] << "M" << start << " " << y << "h" << x - start
                                    << "v1h-" << x - start << "z";
            }
        }
        for (const auto& [ink, path] : paths)
            out << "<path fill=\"" << colors.at(ink) << "\" d=\"" << path.str() << "\"/>\n";
    }

private:
    std::array<std::array<char, Width>, Height> pixels{};
};

using Pixels = PixelGrid<width, height>;
using TrailPixels = PixelGrid<120, 34>;

enum class Face { calm, blink, smile, delight, bite, chew, sip, surprised, sleep, focus, burp };
enum class Item { none, burger, bittenBurger, cola, console, pillow };

constexpr int floorLine = 54;
constexpr int shoulderLine = 33;
constexpr int coatTop = 28;
constexpr int hipLine = 46;
constexpr int hemLine = 50;
constexpr int strideLength = 4;

Pixels portrait(bool side, Face expression) {
    Pixels art;
    // The official portrait stays round through the muzzle, with broad lower cheeks.
    art.stamp(side ? 4 : 3, 4, {"....ggggg....","..gghhhlhgg..",".ghhhhlhhftg.",
        "ghhhhlhhfhttg","ghhhfggghfhtg","ghhfgtgtgtgtt","ghfgtsstgftts",
        "gfgggsttgftts",".fggtttggfts.","..ggttggfts..","...gtgftts...","....tgfts....",
        ".....tts.....","......ts....."});
    art.stamp(side ? 32 : 33, 4, {"...gggg....",".gghhhfgg..","ghhhhhhftg.",
        "ghhlhfhfttg","ghhffgfftts","ghfgtggftts","gfgsttgftts","gfgtttfgtts",
        ".tggttggts.","..tggttts..","..tgftts...","...ttts....","....ts....."});
    const std::vector<Row> face = side
        ? std::vector<Row>{{21,28},{17,31},{16,33},{14,35},{13,36},{12,37},{11,38},{11,39},
            {10,40},{10,40},{9,40},{9,41},{9,41},{8,41},{8,42},{8,42},{8,42},{8,42},
            {8,42},{9,42},{9,42},{9,41},{10,41},{10,40},{11,39},{12,38},{13,37},
            {15,36},{17,34},{19,32}}
        : std::vector<Row>{{20,28},{16,31},{14,33},{13,35},{12,36},{11,37},{10,37},{10,38},
            {9,38},{9,39},{8,39},{8,39},{8,39},{8,40},{8,40},{7,40},{7,40},{7,40},
            {7,40},{7,40},{7,40},{8,39},{8,39},{9,38},{9,37},{10,37},{11,36},
            {12,35},{14,33},{17,31}};
    art.silhouette(3, face, 't', [side](int x, int y, int lo, int hi) {
        if (y >= 28 && x >= hi - 2) return 's';
        if (y >= 30 || x >= hi - 3 || (y >= (side ? 15 : 24) && x <= lo + 2)) return 'g';
        if (x + y / 2 <= 25 && y <= 16) return 'h';
        return 'f';
    });

    // Upper planes use wide clusters; concentrated dark pixels define the features.
    art.stamp(13, 6, {"......lllllhh........","....lllhhhlhhhh......",
                       "...llhhhlhhhhhh......","..lhhhhhhhhhhh.......",
                       ".hhhlhhhhhh..........",".hhhhhhhh............"});
    art.stamp(13, 13, {"..hh...hh.","hhhhh.....","hhhh......",".hh......."});
    art.stamp(26, 10, {"hh...","hhh..",".hh..",".h..."});
    art.stamp(side ? 13 : 10, 20, {"...rr....","..rrrrr..",".rprrrrg.","rrprrrgff",
                                  ".rrrrgff.","..rgff..."});
    art.stamp(side ? 35 : 31, 20, {".rrr....","rrrrr...","rrprrg..","rprrrgf.",
                                  ".rrrgf..","..ggf..."});

    const int nose = side ? 29 : 23;
    const int leftEye = side ? 23 : 19;
    const int rightEye = side ? 33 : 28;
    art.stamp(nose - 1, 21, {".pn.","nepn",".nn."});
    switch (expression) {
        case Face::bite:
        case Face::surprised:
        case Face::burp:
            art.stamp(nose - 2, 24, {".nnn.","nrrpn",".nnn."}); break;
        case Face::chew:
            art.stamp(nose - 2, 24, {".n.n.","..nn."}); break;
        case Face::sip:
        case Face::sleep:
            art.stamp(nose - 1, 24, {".n.","nrn",".n."}); break;
        case Face::smile:
            art.stamp(nose - 3, 24, {"n.....n",".nrrrn.","..nnn.."}); break;
        case Face::delight:
            art.stamp(nose - 3, 24, {"n.....n",".nnnnn.","..nrn..","...n..."}); break;
        default:
            art.stamp(nose - 3, 24, {"n.....n",".n...n.","..nnn.."}); break;
    }

    // Small rose rims follow the eye sockets and leave the nose and smile open.
    for (const int x : {leftEye, rightEye}) {
        art.stamp(x - 3, 16, {"..QQQ..",".Q...J.","Q.....J",".......",
                              "J.....Q",".J...Q.","..JJQ.."});
        if (expression == Face::blink || expression == Face::sleep) art.stamp(x - 1, 20, {"n.n",".n."});
        else if (expression == Face::smile || expression == Face::delight || expression == Face::chew)
            art.stamp(x - 1, 19, {".n.","n.n"});
        else {
            const bool far = side && x == rightEye;
            art.stamp(x - (far ? 1 : 2), 17, far
                ? std::initializer_list<std::string>{".ww.","wnkw","wwkw","wnkw",".kw.",".w.."}
                : std::initializer_list<std::string>{"..ww.",".wnkw","wnwkw","wnkkw",".nkkw","..ww."});
            if (expression == Face::focus) art.dot(x + 1,19,'k');
        }
    }
    art.span(18, leftEye + 4, rightEye - 4, 'Q');
    art.span(18, leftEye - 6, leftEye - 4, 'Q');
    art.span(18, rightEye + 4, rightEye + 6, 'Q');

    // A tucked ribbon adds a little satin highlight near the ear.
    art.stamp(7, 5, {"..N...N..",".NQN.NJN.","NKQJNNQN.","NQQJQJQN.",
                     ".NNQJNJ..","...NJ.N..","...NN...."});
    return art;
}

Pixels coat(bool side, int sway) {
    Pixels art;
    std::vector<Row> jacket = side
        ? std::vector<Row>{{18,31},{17,32},{16,33},{15,34},{15,34},{15,34},{14,34},{13,34},{13,34},{13,34},
            {13,34},{13,34},{13,34},{13,34},{14,34},{14,34},{13,34},{12,34},{12,34},{11,34},
            {11,34},{12,33},{13,32}}
        : std::vector<Row>{{18,29},{16,31},{15,32},{14,33},{14,33},{14,33},{13,34},{12,35},{12,35},{12,35},
            {12,35},{12,35},{12,35},{12,35},{13,34},{13,34},{12,35},{12,35},{12,35},{11,36},
            {11,36},{12,35},{14,33}};
    const auto shift = [sway](int y) { return int(std::lround(sway * (y - 42) / 8.0)); };
    for (int y = 43; y <= hemLine; ++y) {
        auto& [left,right] = jacket[y - coatTop];
        left += shift(y);
        right += shift(y);
    }
    art.silhouette(coatTop, jacket, 'O', [](int x, int y, int lo, int hi) {
        if (x >= hi - 1 || (y >= 47 && x >= hi - 3)) return 'M';
        return y <= 32 || x <= lo + 2 || y >= hemLine - 1 || x >= hi - 3 ? 'W' : 'F';
    });
    if (side) {
        art.stamp(25, 33, {".WCI..","..WMI.","..MFIO","...MIO","...MIO","...MIO","...MIO","...MIO",
                          "...MIO","..MMIO"});
        art.dots({{22,33},{23,34},{24,35},{24,36},{24,37},{25,38},{26,38},{27,37}}, 'T');
        art.stamp(24, 38, {".TT.","TZCT",".TT."});
        art.stamp(18, 41, {".WWW.","FWMM.",".MM.."});
    } else {
        art.stamp(17, 33, {"..FFMIWFW....","...MFIFWM....","...MFIFWM....","...WMIIMW....",
                           "....MIIIM....","....MIIIM....","....MIIIM....","....MIIIM....",
                           "...WMIIIMW...","...WMIIMMW..."});
        art.dots({{17,33},{17,34},{17,35},{17,36},{18,37},{19,38},{20,38},{21,37},{21,36}}, 'T');
        art.dots({{28,33},{29,34},{29,35},{29,36},{29,37}}, 'T');
        art.stamp(27, 37, {".TT.","TZCT",".TT."});
        art.stamp(14, 40, {"..WW..",".FWM..",".WWMO.","..MM.."});
        art.stamp(28, 41, {".WM..",".FWMO",".MMO.","..M.."});
    }
    for (int y = 43; y < hemLine; ++y) {
        const int x = (side ? 29 : 24) + shift(y);
        art.span(y, x, x + 1, 'W');
        art.dot(x, y, y == 44 || y == 47 ? 'S' : 'M');
    }
    art.dot((side ? 29 : 24) + shift(hemLine), hemLine, 0);
    return art;
}


Point plus(Point a, Point b) { return {a.first + b.first, a.second + b.second}; }

Point between(Point a, Point b, double t) {
    return {int(std::lround(a.first + (b.first - a.first) * t)),
            int(std::lround(a.second + (b.second - a.second) * t))};
}

void stroke(Pixels& art, Point a, Point b, int radius, char ink) {
    const int count = std::max({1, std::abs(b.first - a.first), std::abs(b.second - a.second)});
    for (int i = 0; i <= count; ++i) {
        const auto [x, y] = between(a, b, double(i) / count);
        for (int dy = -radius; dy <= radius; ++dy) {
            const int taper = radius - std::abs(dy) / 2;
            if (y + dy >= 0 && y + dy < height)
                art.span(y + dy, std::max(0, x - taper), std::min(width - 1, x + taper), ink);
        }
    }
}

Point elbow(Point shoulder, Point hand, int bend) {
    const double dx = hand.first - shoulder.first, dy = hand.second - shoulder.second;
    const double length = std::max(1.0, std::hypot(dx, dy));
    if (length > 12.25) throw std::runtime_error("Wrist outside arm reach");
    // The hanging arm relaxes. Raising the paw brings the bend back gradually.
    const double flex = std::clamp((10.0 - dy) / 8.0, .25, 1.0);
    const double arc = bend * flex * std::sqrt(std::max(0.0, 36 - length * length / 4)) / length;
    return {int(std::lround((shoulder.first + hand.first) / 2.0 - dy * arc)),
            int(std::lround((shoulder.second + hand.second) / 2.0 + dx * arc))};
}

void sleeve(Pixels& art, Point shoulder, Point hand, int bend, bool shaded) {
    const Point joint = elbow(shoulder, hand, bend);
    const Point wrist = between(joint, hand, .82);
    const std::array<Point, 4> seam{
        shoulder, between(shoulder, joint, .74), between(joint, wrist, .30), wrist
    };
    // The two parts of the arm share one cloth outline and a rounded fold.
    for (std::size_t i = 1; i < seam.size(); ++i)
        stroke(art, seam[i - 1], seam[i], i < 3 ? 3 : 2, 'S');
    for (std::size_t i = 1; i < seam.size(); ++i)
        stroke(art, seam[i - 1], seam[i], i < 3 ? 2 : 1, shaded ? 'M' : 'W');
    for (std::size_t i = 1; i < seam.size(); ++i)
        stroke(art, plus(seam[i - 1], {-1,0}), plus(seam[i], {-1,0}), 1, shaded ? 'W' : 'F');
    stroke(art, between(seam[2], wrist, .82), wrist, 1, 'M');
}

void paw(Pixels& art, Point hand, bool shaded) {
    art.stamp(hand.first - 2, hand.second - 2, shaded
        ? std::initializer_list<std::string>{".sss.","sgtts","sgtso","ogtso",".ooo."}
        : std::initializer_list<std::string>{".sss.","shhgs","shfto","ogfto",".ooo."});
}

void trouserLeg(Pixels& art, Point hip, Point foot, bool side, bool shaded) {
    const Point cuff{foot.first, foot.second - 4};
    for (int y = hip.second; y <= cuff.second; ++y) {
        const int x = between(hip, cuff, double(y - hip.second) / std::max(1, cuff.second - hip.second)).first;
        const int half = y == cuff.second ? 2 : 3;
        art.span(y, x - half, x + half, 'E');
        art.span(y, x - half + 1, x + half - 1, shaded ? '7' : 'c');
        if (!shaded) art.dot(x - 1, y, 'z');
    }
    if (side) {
        art.stamp(foot.first - 3, cuff.second + 1, shaded
            ? std::initializer_list<std::string>{".oooto..","otggtso.","ogggtsto",".oooooo."}
            : std::initializer_list<std::string>{".sooto..","ohhhfso.","ofhhfgto",".oooooo."});
    } else {
        art.stamp(foot.first - 4, cuff.second + 1, {"..sotoo..",".ohhhgto.","ohhhfggto",".ooooooo."});
    }
}

Pixels food(Item item, Point anchor, int moment) {
    Pixels art;
    const auto [x, y] = anchor;
    if (item == Item::burger || item == Item::bittenBurger) {
        art.stamp(x - 8, y, {".....uuuuuu.....","...uujjjjjjuu...","..ujjjljjjjjju..",
                            ".ujjljjjjjjjjuu.","uuujjjjjjjjjjyuu","uuyyyyyyyyyyyyuu",
                            ".HGGGHHHHGGGGH..",".qqqRRqqqqRqqq..","..uyjjjjjjjyu...",
                            "...uuuuuuuuu...."});
        if (item == Item::bittenBurger) {
            for (int drop = 0; drop < 4; ++drop) art.span(y + drop, x + 1, x + 7 - drop, 0);
            art.dots({{x,y+1},{x,y+2},{x+1,y+3},{x+2,y+4},{x+3,y+4},{x+4,y+3}}, 'u');
        }
    } else if (item == Item::cola) {
        art.stamp(x - 6, y, {"..OOOOOOOOO.","OFFFFFWFWFWO","OMMMMMMMMMSO",".XPPPVVVXXX.",
                            ".XPVVVVVVX..",".XPVwVVVVX..","..XVwwVVVX..","..XVwVVVX...",
                            "..XVVVVVX...","...XPPPXX...","....XXXX...."});
        art.stamp(x - 6, y - 8, {"DDD...","..ID..","...ID.","....ID",".....D",".....I",".....D",".....D"});
    } else if (item == Item::console) {
        art.stamp(x - 10, y, {"..EEEEEEEEEEEEEEEE..",".ELLLLLLLLLLLLLLLLE.",
                              "ELEEEETTTTTTEEGEEELE","ELELEETTTTTTEEEELELE",
                              "ELEEEETTTTTTEELYEELE",".ELLLDDDDDDDDDLLLLE.",
                              "..EEEEEEEEEEEEEEEE.."});
        art.stamp(x - 3 + moment % 6, y + (moment % 3 == 0 ? 2 : 3), {".Z","ZZ"});
    } else if (item == Item::pillow) {
        art.stamp(x - 9, y, {"..vvvvvvvvvvv.....",".vdddddbaaabav....","vdbddddbbbabav....",
                             "vdabdddddbbabav...","vbbddddddbbbaav...","vbdvdddbbabaaav...",
                             ".vbabbbabbaaav....","..vvbaaaaaavv.....","....vvvvvvv......."});
    }
    return art;
}

struct Pose {
    bool side = false;
    int x = 0;
    int lift = 0;
    int chest = 0;
    int hem = 0;
    Point nod{};
    Face face = Face::calm;
    Point leftHand{13, 43};
    Point rightHand{34, 43};
    Point leftFoot{18, floorLine};
    Point rightFoot{29, floorLine};
    Item item = Item::none;
    Point prop{24, 34};
    int propBottom = height;
    int rightBend = -1;
};

Pose sidePose() {
    Pose pose;
    pose.side = true;
    pose.leftHand = {30, 40};
    pose.rightHand = {16, 42};
    pose.leftFoot = {21, floorLine};
    pose.rightFoot = {25, floorLine};
    return pose;
}

Pixels actor(const Pose& pose, int moment) {
    Pixels body;
    const Point leftShoulder{pose.side ? 28 : 15, shoulderLine + pose.chest};
    const Point rightShoulder{pose.side ? 18 : 32, shoulderLine + pose.chest};
    const auto handL = plus(pose.leftHand, {0, pose.chest});
    const auto handR = plus(pose.rightHand, {0, pose.chest});
    if (pose.side) {
        Pixels tail;
        tail.stamp(9, 41, {"..oo..",".ohgo.","ofgto.",".tso..","..oo.."});
        body.over(tail, 0, pose.chest);
    }
    body.stamp(15, hipLine - 2 + pose.chest, {"..EEEEEEEEEEEEEE..",".EccccccccccccccE.",
                                            "EccccccccccccccccE",".EccccccccccccccE."});
    trouserLeg(body, {pose.side ? 25 : 18, hipLine + pose.chest}, pose.leftFoot, pose.side, pose.side);
    trouserLeg(body, {pose.side ? 21 : 29, hipLine + pose.chest}, pose.rightFoot, pose.side, false);
    if (pose.side) {
        sleeve(body, leftShoulder, handL, 1, true);
        paw(body, handL, true);
    }
    body.over(coat(pose.side, pose.hem), 0, pose.chest);
    if (!pose.side) sleeve(body, leftShoulder, handL, 1, false);
    sleeve(body, rightShoulder, handR, pose.rightBend, false);
    // The head overlaps the coat and sleeve roots even on the upward glance.
    body.over(portrait(pose.side, pose.face), pose.nod.first, pose.chest + pose.nod.second);
    auto prop = food(pose.item, pose.prop, moment);
    for (int y = pose.propBottom; y < height; ++y) prop.span(y, 0, width - 1, 0);
    body.over(prop, 0, pose.chest);
    if (!pose.side) paw(body, handL, false);
    paw(body, handR, pose.side);
    Pixels frame;
    frame.over(body, pose.x, pose.lift);
    return frame;
}

void hold(Pose& p, Item item, int x, int y, double grip = 1) {
    p.item = item;
    p.prop = {x, y};
    if (item == Item::cola) {
        p.leftHand = between(p.leftHand, {x - 7, y + 2}, grip);
        p.rightHand = between(p.rightHand, {x + 4, y + 3}, grip);
        p.rightBend = 1;
    } else {
        const int reach = item == Item::console ? 9 : 7;
        const int gripOffset = item == Item::console ? 4 : 5;
        p.leftHand = between(p.leftHand, {x - reach, y + gripOffset}, grip);
        p.rightHand = between(p.rightHand, {x + reach, y + gripOffset}, grip);
    }
}

Pixels parcel(bool shut, bool front) {
    Pixels art;
    if (!front) {
        art.stamp(9, 34, {"...000000000000000000000000...","..03333333333333333333333330..",
                          ".0333333333333333333333333330.","004433333333333333333333334400",
                          "011111111111111111111111111110","011111111111111111111111111110"});
        if (!shut) {
            art.stamp(6, 31, {"...00...", "..0440..", ".04440..","044440..",".03330..","..030..."});
            art.stamp(36, 31, {"...00...", "..0440..", "..04440.","..044440","..03330.","..030..."});
        }
        return art;
    }
    const std::vector<Row> panel{{9,38},{9,38},{9,38},{9,38},{9,38},{9,38},{9,38},{9,38},
                                 {9,38},{9,38},{9,38},{10,37},{10,37},{11,36},{12,35}};
    art.silhouette(39, panel, '0', [](int x, int y, int, int hi) {
        if (x >= hi - 2 || y >= 51) return '1';
        if (x >= hi - 4 || y >= 48) return '2';
        return y < 42 ? '4' : '3';
    });
    for (int y = 40; y <= 51; ++y) art.span(y, 23, 25, y >= 49 ? '2' : '4');
    art.stamp(13, 43, {"44444","45Q54","4QJQ4","45Q54","44444"});
    if (shut) {
        art.span(37, 11, 36, '0');
        art.span(38, 9, 38, '3');
        art.span(39, 10, 37, '4');
        art.span(38, 23, 25, '5');
    }
    return art;
}

void heart(Pixels& art, int x, int y) {
    art.stamp(x, y, {".N.N.","NQKQN","NQJJN",".NJN.","..N.."});
}

void dust(Pixels& art, int x, int y, int age) {
    for (const int direction : {-1, 1}) {
        art.dot(x + direction * (8 + age), y - 1 - age, age < 2 ? 'l' : 'j');
        art.dot(x + direction * (10 + age), y - 2, 'y');
    }
}

void beginGroup(std::ostream& out, const std::string& name) {
    out << "<g class=\"" << name << "\">\n";
}

void drawLandscape(std::ostream& out) {
    struct Scheme {
        std::string name, grass, edge, accent, canopy, shade, stem, detail, glow;
    };
    const Scheme schemes[] = {
        {"spring", "#aecb8e", "#829966", "#e7efb9", "#ecc3d3", "#d69fb7", "#997862", "#b85c82", "#f894b2"},
        {"summer", "#9fc493", "#719778", "#d8e9b7", "#b9d9b7", "#91baa5", "#7b8d65", "#b97238", "#f9cd76"},
        {"autumn", "#dcb278", "#a67c55", "#f3d09d", "#edc189", "#d6a171", "#a07554", "#ae6145", "#eeaa6d"},
        {"winter", "#d8e8ee", "#99b7c5", "#fbfcf7", "#c1d9de", "#8fadb9", "#80a1ac", "#6b93a9", "#f3f9f8"},
    };
    out << "<svg class=\"pb-landscape\" x=\"0\" y=\"0\" width=\"120\" height=\"34\" viewBox=\"0 0 120 34\" "
        << "preserveAspectRatio=\"none\" opacity=\"0\" aria-hidden=\"true\" shape-rendering=\"crispEdges\">\n"
        << "<defs><radialGradient id=\"pb-trail-light\"><stop offset=\"0\" stop-color=\"var(--pb-light,#ffeec1)\" stop-opacity=\".9\"/>"
        << "<stop offset=\"1\" stop-color=\"var(--pb-light,#ffeec1)\" stop-opacity=\"0\"/></radialGradient>"
        << "<linearGradient id=\"pb-trail-ray\" x2=\".75\" y2=\"1\"><stop stop-color=\"#fff8df\" stop-opacity=\".72\"/>"
        << "<stop offset=\"1\" stop-color=\"var(--pb-light,#ffeec1)\" stop-opacity=\"0\"/></linearGradient></defs>\n"
        << "<ellipse cx=\"34\" cy=\"12\" rx=\"42\" ry=\"19\" fill=\"url(#pb-trail-light)\" shape-rendering=\"geometricPrecision\"/>\n";
    for (const auto& season : schemes) {
        const std::map<char, std::string> colors{{'g', season.grass}, {'e', season.edge}, {'a', season.accent},
            {'c',season.canopy}, {'s',season.shade}, {'t',season.stem}, {'d',season.detail}, {'h',season.glow}};
        beginGroup(out, "pb-season pb-season-" + season.name);
        TrailPixels background;
        background.stamp(55, 17, {".......................cccc.............", "...........ccc.......ccccccc...........",
            ".........cccccc....ccccccccccc.........", "........ccccccccccccccccccccccc........",
            ".......cccccccccccccccccccccccccc......", "...cccccccccccccccccccccccccccccccc....",
            ".ccccccccccccccccccccccccccccccccccc...", "ccccccccccccccccccccccccccccccccccccc..",
            ".cccccccccsssscccccssssccccccccccccccc.", "..ccccccssssssscccssssssscccccccccc...."});
        if (season.name == "winter") {
            background.stamp(15, 5, {".........c.........","........cac........",".......caaac.......",
                "......caaaaac......",".......ssss........","......caaaaac......",".....caaaaaaac.....",
                "....caaaaaaaaac....","......sssssss......","....ccaaaaaaacc....","...caaaaaaaaaaac...",
                "..caaaaaaaaaaaaac..",".ccaaaaaaaaaaaaacc.","..sssssssssssssss..",".......tttt........",
                ".......tttt........",".......tttt........"});
            background.stamp(91, 13, {".......c.......","......cac......",".....caaac.....","....caaaaac....",
                ".....sssss.....","....caaaaac....","...caaaaaaac...","..caaaaaaaaac..","...sssssssss...",
                "......ttt......","......ttt......","......ttt......"});
        } else {
            background.stamp(13, 6, {"........ccccccc........",".....ccccccccccc.......","...cccaccccccccccc.....",
                "..ccccccccccccccccc....",".cccccccccccccccccccc..","cccccccccccccccccccccc.",
                "cccscccccccccccccccccc.","ccssccccccccccccccsccc.",".cssccccccccssccccsscc.",
                "..sssscccccsssscsssss..","...ssssssssssssssss....",".....ssstttsssss.......",
                ".......tttttt..........",".......t.tt............",".........tt............",
                ".........tt............",".........tt............",".........tt............"});
            if (season.name == "summer") {
                background.stamp(93, 7, {"..hhh..",".hhhhh.","hhhhhhh","hhhhhhh",".hhhhh.","..hhh.."});
                background.span(23, 89, 105, 'c');
                background.span(24, 85, 111, 's');
                background.span(25, 92, 108, 'a');
            } else {
                background.stamp(89, 15, {"......cccc.....","...cccccccccc..",".ccccccccccccc.",
                    "ccccccsccccccsc",".scccsssccccss.","..sssssssssss..",".....ttt.......",
                    ".....ttt.......",".....ttt......."});
            }
        }
        out << "<g opacity=\".83\">\n";
        background.write(out, colors);
        out << "</g>\n";

        TrailPixels ground;
        const Row bank[]{{16,107},{8,111},{5,115},{6,113},{11,108},{18,101},{31,91}};
        for (int i = 0; i < 7; ++i) ground.span(26 + i, bank[i].first, bank[i].second, i < 4 ? 'g' : 'e');
        ground.span(26, 23, 39, 'a'); ground.span(26, 59, 67, 'a'); ground.span(27, 94, 107, 'a');
        ground.span(27, 9, 17, 'a'); ground.span(26, 44, 51, 'a'); ground.span(26, 79, 85, 'a');
        ground.span(28, 15, 23, 'a'); ground.span(29, 49, 57, 'a'); ground.span(29, 74, 80, 'a');
        ground.dots({{16,30},{25,29},{29,29},{42,27},{79,27},{89,29},{104,28}}, 'e');
        ground.dots({{10,28},{31,29},{36,30},{46,28},{54,30},{66,29},{70,30},{97,29},{109,27}}, 'a');
        ground.span(31, 24, 29, 'g'); ground.span(32, 45, 50, 'g'); ground.span(30, 84, 93, 'g');
        ground.stamp(33, 23, {"....e..","e...e.e",".e.ee.e","..e.e.."});
        ground.stamp(78, 23, {".e.....",".e.e..e",".e.e.e.","..eee.."});
        ground.write(out, colors);
        out << "<path fill=\"var(--pb-shadow,#5e6d55)\" opacity=\".19\" "
            << "d=\"M23 25h5v1h5v1h7v1h-6v1H26v-1h-5v-1h2zM96 25h3v1h5v1h6v1h-9v-1h-5z\"/>\n";
        out << "<path fill=\"var(--pb-shadow,#5e6d55)\" opacity=\".24\" "
            << "d=\"M11 30h22v1h-9v1h-6v-1h-7zM46 31h43v1H51v1H31v-1h15zM97 30h11v1h-7v1h-8v-1h4z\"/>\n"
            << "<path d=\"M23 1h9l22 28H42zM38 3h4l19 25h-6z\" fill=\"url(#pb-trail-ray)\" "
            << "shape-rendering=\"geometricPrecision\" opacity=\".45\"/>\n";
        out << "<path class=\"pb-glints\" fill=\"var(--pb-light,#ffeec1)\" opacity=\".58\" "
            << "d=\"M12 14h1v1h-1zM39 13h2v1h-2zM45 20h1v1h-1zM77 15h1v1h-1zM107 22h1v1h-1z\"/>\n";

        beginGroup(out, "pb-landmark");
        TrailPixels discovery;
        if (season.name == "spring") {
            discovery.stamp(55, 8, {"....dd.....","...dhhd....",".ddhhhhdd..","dhhhhhhhhd.",
                "dhhhahhhhd.",".ddhahhdd..","...dhhd....","....dd.....","....ee.....",
                "....ee..e..",".e..ee.ea..",".ea.eeea...","..eaeea....","...eee.....",
                "....ee.....","....ee.....","....ee.....","....ee.....","....ee.....","...eee....."});
        } else if (season.name == "summer") {
            discovery.stamp(55, 16, {".dd....dd.","dhhd..dhhd","dhhhd.dhhd",".dhhhtthd.","..ddttdd..","....t....."});
        } else if (season.name == "autumn") {
            discovery.stamp(55, 16, {"....d.....","..d.d.d...","..dhdhd.d.",".dhhhhhd..",
                "dhhhhhhhhd",".ddhhdhdd.","...dddd...","....dt....",".....t...."});
        } else {
            discovery.stamp(55, 10, {"....d.....","..d.d.d...","...dhd....",".d.dhd.d..",
                "dddhhhddd.",".d.dhd.d..","...dhd....","..d.d.d...","....d....."});
        }
        beginGroup(out, "pb-discovery");
        discovery.write(out, colors);
        out << "</g>\n</g>\n</g>\n";
    }
    out << "</svg>\n";
}

double blend(double seconds, double from, double to) {
    const double t = std::clamp((seconds - from) / (to - from), 0.0, 1.0);
    return t * t * (3 - 2 * t);
}

int pixel(double value) { return int(std::lround(value)); }

void cuddleHands(Pose& pose, double tuck, double beat) {
    // The reference holds both paws under the cheeks and lets one lead the other.
    pose.leftHand = between(pose.leftHand, {21 + pixel(beat), 37 + pixel(3 * beat)}, tuck);
    pose.rightHand = between(pose.rightHand, {29 + pixel(beat), 36 - pixel(3 * beat)}, tuck);
    pose.nod = {pixel(1.5 * tuck * beat), pixel(tuck * (1.3 - .45 * std::abs(beat)))};
}

Pixels pivot(std::string_view scene, double seconds) {
    const bool turn = scene == "turn" || scene == "turnLeft";
    const bool departing = scene == "stepOff" || scene == "stepOffLeft";
    const bool left = scene.find("Left") != std::string_view::npos;
    const bool mirrored = left != (turn && seconds > .20);
    const double t = std::clamp(seconds / .20, 0.0, turn ? 2.0 : 1.0);
    const double towardFront = turn ? 1 - std::abs(t - 1) : departing ? 1 - t : t;
    Pose pose;
    if (towardFront < .45) {
        pose = sidePose();
        const double step = blend(towardFront, 0, .45);
        pose.rightFoot = between(pose.rightFoot, {24, floorLine - 2}, step);
        pose.rightHand = between(pose.rightHand, {18, 41}, step);
        pose.hem = -pixel(step);
    } else {
        const double settle = 1 - blend(towardFront, .45, 1);
        pose.leftFoot.first += pixel(2 * settle);
        pose.rightFoot.first -= pixel(2 * settle);
        auto& foot = mirrored ? pose.leftFoot : pose.rightFoot;
        foot.second -= pixel(settle);
        pose.hem = pixel((mirrored ? 1 : -1) * settle);
    }
    const Pixels drawing = actor(pose, 0);
    if (!pose.side || !mirrored) return drawing;
    Pixels result;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            result.dot(width - 1 - x, y, drawing.at(x, y));
    return result;
}

void wave(Pose& pose, double t) {
    const Point resting{34,43}, stretched{43,38};
    const Point raised{40 + pixel(2 * std::sin(t * 16)), 27 + pixel(std::cos(t * 16))};
    if (t < .15 || t >= .62) {
        pose.rightHand = between(resting, stretched, blend(t, 0, .15) - blend(t, .62, .76));
    } else {
        pose.rightHand = between(stretched, raised, blend(t, .15, .28) - blend(t, .51, .62));
        pose.rightBend = 1;
    }
}

Pixels drawFrame(std::string_view scene, int frame, double seconds) {
    if (scene == "stepOff" || scene == "stepOffLeft" || scene == "turn" || scene == "turnLeft" ||
        scene == "settle" || scene == "settleLeft") return pivot(scene, seconds);
    Pose pose;
    if (scene == "walk") {
        pose = sidePose();
        const int stride[] = {2,2,1,1,0,0,-1,-1,-2,-2,-1,-1,0,1,1,2};
        const int rise[] = {0,0,0,0,0,0,0,0,0,1,1,2,2,2,1,1};
        const int left = (frame + 8) % 16, right = frame % 16;
        pose.leftFoot = {23 + stride[left], floorLine - rise[left]};
        pose.rightFoot = {23 + stride[right], floorLine - rise[right]};
        pose.leftHand = {29 - stride[left] / 2, 41 - std::abs(stride[left]) / 2};
        pose.rightHand = {18 - stride[right], 43 - std::abs(stride[right]) / 2};
        pose.chest = frame % 8 >= 2 && frame % 8 <= 5 ? -1 : 0;
        pose.hem = -std::max(rise[left], rise[right]);
    } else if (scene == "inspect" || scene == "inspectUp") {
        pose = sidePose();
        const double lean = blend(seconds,.28,1.06) - blend(seconds,1.78,2.60);
        const double sniff = blend(seconds,1.08,1.26) - blend(seconds,1.38,1.6);
        pose.chest = pixel(lean);
        pose.nod = {pixel(2 * lean), pixel((scene == "inspect" ? 2 + sniff : -2) * lean)};
        pose.rightHand = between(pose.rightHand, {25,40}, lean);
        pose.hem = pixel(lean);
        pose.face = seconds > .86 && seconds < 2.12 ? Face::smile : Face::calm;
    } else if (scene == "pat") {
        const double tuck = blend(seconds,.04,.47) - blend(seconds,1.87,2.36);
        cuddleHands(pose, tuck, std::cos((seconds - .45) * 3.55));
        const double gather = blend(seconds,1.16,1.50) - blend(seconds,1.69,2.0);
        pose.leftHand = between(pose.leftHand, {21,33}, gather);
        pose.rightHand = between(pose.rightHand, {26,33}, gather);
        if (seconds > .48 && seconds < 1.13) pose.face = Face::delight;
    } else if (scene == "feed") {
        if (seconds < 4.42) {
            const double grip = blend(seconds,.08,.51) - blend(seconds,3.83,4.39);
            const double raise = blend(seconds,.52,1.72);
            const double lower = blend(seconds,2.08,2.72) - blend(seconds,3.14,3.70);
            const bool bitten = seconds >= 2.08;
            hold(pose, bitten ? Item::bittenBurger : Item::burger, 24,
                 39 - pixel(15 * raise) + pixel(9 * lower), grip);
            pose.propBottom = 39 + pixel(17 * blend(seconds,.84,1.46));
            if (seconds >= 3.83) pose.item = Item::none;
            const bool bite = (seconds >= 1.62 && !bitten) || (seconds >= 3.49 && seconds < 3.83);
            if (bite) pose.face = Face::bite;
            else if (bitten) pose.face = std::sin(seconds * 25) > 0 ? Face::chew : Face::smile;
            pose.nod.second = pixel(blend(seconds,1.76,2.03) - blend(seconds,2.18,2.54));
        } else if (seconds >= 4.52 && seconds < 7.96) {
            const double grip = blend(seconds,4.52,4.96) - blend(seconds,7.52,7.96);
            const double sip = blend(seconds,5.02,5.88) - blend(seconds,6.7,7.43);
            hold(pose, Item::cola, 29 + pixel(1 - sip), 40 - pixel(7 * sip), grip);
            const double clearPocket = blend(seconds,4.85,5.57) - blend(seconds,7.20,7.96);
            pose.propBottom = 32 + pixel(24 * clearPocket);
            if (sip > .91) pose.face = Face::sip;
            else if (seconds > 6.80) pose.face = Face::smile;
        } else if (seconds >= 7.96 && seconds < 9.70) {
            const double hug = blend(seconds,7.96,8.36) - blend(seconds,9.21,9.7);
            const double hiccup = blend(seconds,8.71,8.90) - blend(seconds,8.91,9.18);
            pose.face = hiccup > .55 ? Face::burp : Face::smile;
            pose.chest = -pixel(hiccup);
            pose.leftHand = between(pose.leftHand, {19,43}, hug);
            pose.rightHand = between(pose.rightHand, {30,43}, hug);
        }
    } else if (scene == "dance") {
        const double energy = blend(seconds,.08,.45) - blend(seconds,2.66,3.16);
        const double beat = std::sin((seconds - .20) * 5.2);
        const double sway = energy * beat;
        cuddleHands(pose, energy, beat);
        const double open = blend(seconds,1.12,1.43) - blend(seconds,1.62,1.90);
        const double hop = blend(seconds,1.26,1.47) - blend(seconds,1.47,1.73);
        const double crouch = blend(seconds,1.02,1.19) - blend(seconds,1.20,1.41);
        const double land = blend(seconds,1.66,1.77) - blend(seconds,1.77,1.96);
        pose.leftHand = between(pose.leftHand, {11,36}, open);
        pose.rightHand = between(pose.rightHand, {37,35}, open);
        if (energy > .35) pose.face = hop > .25 ? Face::delight : Face::smile;
        pose.leftFoot.second -= pixel(2 * std::max(0.0, sway));
        pose.rightFoot.second -= pixel(2 * std::max(0.0, -sway));
        pose.lift = -pixel(3 * hop);
        pose.chest = pixel(crouch + land - std::abs(sway) * .6);
        pose.hem = pixel(2 * sway);
    } else if (scene == "sleep" && seconds > .28 && seconds < 9.72) {
        const double lean = blend(seconds,.32,1.35) - blend(seconds,8.95,9.54);
        const double grip = blend(seconds,.28,.9) - blend(seconds,9.12,9.72);
        hold(pose, Item::pillow, 24, 39 - pixel(4 * lean), grip);
        pose.propBottom = 39 + pixel(17 * (blend(seconds,.43,1.16) - blend(seconds,9.10,9.72)));
        pose.nod = {pixel(-2 * lean), pixel(2 * lean)};
        pose.chest = pixel(lean * (.7 + .3 * std::sin(seconds * 3)));
        pose.face = lean > .38 ? Face::sleep : Face::blink;
    } else if (scene == "game" && seconds > .23 && seconds < 7.85) {
        const double raise = blend(seconds,.34,1.01) - blend(seconds,7.12,7.85);
        hold(pose, Item::console, 24, 40 - pixel(4 * raise),
             blend(seconds,.23,.75) - blend(seconds,7.27,7.85));
        pose.propBottom = 40 + pixel(16 * raise);
        pose.leftHand.second -= pixel(raise * std::max(0.0, std::sin(seconds * 15)));
        pose.rightHand.second -= pixel(raise * std::max(0.0, std::sin(seconds * 19 + 1)));
        pose.nod.second = seconds < 5.80 ? pixel(raise) : 0;
        if (raise > .45) pose.face = seconds > 5.68 ? Face::smile : Face::focus;
        pose.lift = -pixel(3 * (blend(seconds,5.80,6.02) - blend(seconds,6.02,6.30)));
        pose.chest = pixel(blend(seconds,6.27,6.38) - blend(seconds,6.38,6.64));
    } else if (scene == "drag") {
        const double kick = std::cos(seconds * 2 * std::acos(-1.0) / .72);
        pose.face = Face::surprised;
        pose.lift = -2;
        pose.leftFoot.second -= pixel(1 + kick);
        pose.rightFoot.second -= pixel(1 - kick);
        pose.leftHand = {11, 39 - pixel(kick)};
        pose.rightHand = {36, 39 + pixel(kick)};
        pose.hem = pixel(2 * kick);
    } else if (scene == "enter" || scene == "exit") {
        const bool entering = scene == "enter";
        const bool box = entering ? seconds < 1.77 : seconds >= .34;
        const bool shut = entering ? seconds < .22 : seconds > 1.42;
        int boxX = 0, flap = 0;
        if (entering) {
            const double jump = blend(seconds,.88,1.31);
            pose.lift = 24 - pixel(7 * blend(seconds,.27,.61) + 20 * jump)
                + pixel(3 * blend(seconds,1.31,1.53));
            pose.x = -pixel(2 * (1 - jump));
            pose.chest = pixel(blend(seconds,1.46,1.55) - blend(seconds,1.60,1.73));
            pose.hem = -pixel(2 * (jump - blend(seconds,1.40,1.70)));
            const double brace = blend(seconds,.50,.73) - blend(seconds,.91,1.14);
            const int rim = std::clamp(39 - pose.lift, 22, 41);
            pose.leftHand = between(pose.leftHand,{14,rim},brace);
            pose.rightHand = between(pose.rightHand,{34,rim},brace);
            if (seconds >= 1.39) wave(pose, seconds - 1.39);
            flap = pixel(14 * blend(seconds,.94,1.38));
            boxX = -pixel(46 * blend(seconds,1.25,1.77));
        } else {
            pose.lift = -pixel(3 * (blend(seconds,.70,.90) - blend(seconds,.90,1.08)))
                + pixel(34 * blend(seconds,.91,1.38));
            pose.x = -pixel(2 * blend(seconds,.89,1.25));
            pose.chest = pixel(blend(seconds,.75,.86) - blend(seconds,.86,1.02));
            pose.hem = pixel(2 * (blend(seconds,.69,.92) - blend(seconds,.92,1.16)));
            wave(pose, seconds);
            flap = 14 - pixel(14 * blend(seconds,.91,1.26));
            boxX = -pixel(46 * (1 - blend(seconds,.34,.82)));
        }
        if (seconds > (entering ? 1.34 : .1) && (!entering || seconds < 2.06)) pose.face = Face::smile;
        Pixels result;
        if (box) result.over(parcel(shut, false), boxX, flap / 2);
        if (!shut || !box) result.over(actor(pose, frame));
        if (box) result.over(parcel(shut, true), boxX, flap);
        if (entering && seconds >= 1.52 && seconds < 1.82) dust(result,24,54,pixel((seconds - 1.52) * 12));
        return result;
    } else if (scene == "rest") {
        const double tuck = blend(seconds,1.25,1.9) - blend(seconds,3.58,4.45);
        cuddleHands(pose, tuck, std::cos((seconds - 1.9) * 1.95));
        if (seconds > 4.6 && seconds < 4.86) pose.face = Face::blink;
    }

    Pixels result = actor(pose, frame);
    if (scene == "walk" && (frame % 8 == 1 || frame % 8 == 2)) {
        result.dot(15 - frame % 8, 54 - frame % 8, 'j');
        result.dot(18 - frame % 8, 54, 'l');
    }
    if (scene == "pat") {
        if (seconds > .25 && seconds < 1.35) heart(result, 39, 11 - pixel(seconds * 5));
        if (seconds > 1.49 && seconds < 2.1)
            heart(result, 35 + pixel((seconds - 1.49) * 12), 21 - pixel((seconds - 1.49) * 12));
    }
    if (scene == "feed" && ((seconds >= 2.07 && seconds < 2.34) || (seconds >= 3.82 && seconds < 4.04))) {
        const double age = seconds < 3 ? seconds - 2.07 : seconds - 3.82;
        result.dots({{29 + pixel(age * 9), 26 + pixel(age * 10)}, {31, 29 + pixel(age * 8)}}, 'j');
    }
    if (scene == "feed" && seconds >= 8.85 && seconds < 9.48) {
        const double age = seconds - 8.85;
        const int x = 28 + pixel(age * 19), y = 25 - pixel(age * 9);
        if (age < .14 || age > .48) result.dots({{x,y},{x + 2,y - 1},{x + 1,y + 2}}, 'D');
        else result.stamp(x, y, {"..DD..",".DIWD.","DIFWID",".DIID.","..DD.."});
    }
    if (scene == "sleep" && pose.face == Face::sleep) {
        const int y = 7 - (frame % 15) / 3;
        result.stamp(38, y, {"CCC","..C",".C.","CCC"});
        result.stamp(43, y + 7, {"CC",".C","CC"});
    }
    if ((scene == "dance" && seconds > .34 && seconds < 2.87) ||
        (scene == "game" && seconds > 5.71 && seconds < 6.59)) {
        const int y = 14 - frame % 9;
        result.stamp(39, y, {"..ZlZ.","..Z.l.","..Z...",".ZZ...","ZZZ..."});
    }
    return result;
}

struct Clip {
    const char* name;
    int duration;
    int frames;
    bool repeat;
    std::vector<int> tiles;
};

std::string base64(const std::vector<unsigned char>& bytes) {
    constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    encoded.reserve((bytes.size() + 2) / 3 * 4);
    for (std::size_t i = 0; i < bytes.size(); i += 3) {
        const unsigned bits = unsigned(bytes[i]) << 16 |
            (i + 1 < bytes.size() ? unsigned(bytes[i+1]) << 8 : 0) |
            (i + 2 < bytes.size() ? unsigned(bytes[i+2]) : 0);
        encoded.push_back(alphabet[bits >> 18]);
        encoded.push_back(alphabet[(bits >> 12) & 63]);
        encoded.push_back(i + 1 < bytes.size() ? alphabet[(bits >> 6) & 63] : '=');
        encoded.push_back(i + 2 < bytes.size() ? alphabet[bits & 63] : '=');
    }
    return encoded;
}

constexpr int columns = 24;

std::string makeAtlas(const std::vector<Pixels>& frames, int rows) {
    png_image image{};
    image.version = PNG_IMAGE_VERSION;
    image.width = columns * width;
    image.height = rows * height;
    image.format = PNG_FORMAT_RGBA_COLORMAP;
    std::vector<unsigned char> colormap{0,0,0,0};
    std::map<char, unsigned char> index{{0,0}};
    for (const auto& [ink, color] : palette) {
        index[ink] = static_cast<unsigned char>(colormap.size() / 4);
        for (int channel = 0; channel < 3; ++channel)
            colormap.push_back(static_cast<unsigned char>(std::stoi(color.substr(1 + channel * 2, 2), nullptr, 16)));
        colormap.push_back(255);
    }
    image.colormap_entries = static_cast<png_uint_32>(colormap.size() / 4);
    std::vector<unsigned char> data(image.width * image.height);
    for (std::size_t i = 0; i < frames.size(); ++i) {
        const int left = (i % columns) * width, top = (i / columns) * height;
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                data[(top + y) * image.width + left + x] = index.at(frames[i].at(x,y));
    }
    png_alloc_size_t size = 0;
    if (!png_image_write_to_memory(&image, nullptr, &size, 0, data.data(), 0, colormap.data()))
        throw std::runtime_error(image.message);
    std::vector<unsigned char> bytes(size);
    if (!png_image_write_to_memory(&image, bytes.data(), &size, 0, data.data(), 0, colormap.data()))
        throw std::runtime_error(image.message);
    bytes.resize(size);
    return base64(bytes);
}

void draw(std::ostream& out) {
    Clip clips[]{
        {"rest",5600,56,true,{}}, {"walk",640,16,true,{}},
        {"stepOff",240,6,false,{}}, {"stepOffLeft",240,6,false,{}},
        {"turn",440,11,false,{}}, {"turnLeft",440,11,false,{}},
        {"settle",240,6,false,{}}, {"settleLeft",240,6,false,{}},
        {"inspect",2700,36,false,{}}, {"inspectUp",2700,36,false,{}},
        {"pat",2400,40,false,{}}, {"feed",10000,160,false,{}}, {"dance",3200,48,false,{}},
        {"sleep",10000,80,false,{}}, {"game",8000,96,false,{}},
        {"enter",2200,44,false,{}}, {"exit",1700,34,false,{}}, {"drag",720,12,true,{}},
    };
    std::map<std::string,int> cells;
    std::vector<Pixels> frames;
    for (auto& clip : clips) {
        for (int i = 0; i < clip.frames; ++i) {
            const auto art = drawFrame(clip.name, i, double(i) * clip.duration / (1000 * clip.frames));
            std::string key;
            key.reserve(width * height);
            for (int y = 0; y < height; ++y)
                for (int x = 0; x < width; ++x) key.push_back(art.at(x,y));
            const auto [it, fresh] = cells.try_emplace(std::move(key), static_cast<int>(frames.size()));
            if (fresh) frames.push_back(art);
            clip.tiles.push_back(it->second);
        }
    }
    const int rows = (frames.size() + columns - 1) / columns;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" class=\"pb-rig\" width=\"48\" height=\"56\" "
        << "viewBox=\"0 0 48 56\" overflow=\"visible\" shape-rendering=\"crispEdges\" aria-hidden=\"true\">\n"
        << "<title>小熊熊 · 奶油像素</title>\n"
        << "<desc>圆腮的奶油色小熊，戴粉色细框眼镜，穿盖住膝盖的白大褂。双爪交替贴近脸颊，然后在胸前合拢。</desc>\n"
        << "<metadata class=\"pb-motion\">{\"columns\":" << columns << ",\"stride\":" << strideLength << ",\"clips\":{";
    bool first = true;
    for (const auto& clip : clips) {
        out << (first ? "" : ",") << '"' << clip.name << "\":[" << clip.duration << ','
            << (clip.repeat ? "true" : "false") << ",[";
        first = false;
        for (std::size_t i = 0; i < clip.tiles.size(); ++i) out << (i ? "," : "") << clip.tiles[i];
        out << "]]";
    }
    out << "}}</metadata>\n"
        << "<defs><radialGradient id=\"pb-ground-soft\"><stop stop-color=\"var(--pb-shadow,#58474a)\" stop-opacity=\".4\"/>"
        << "<stop offset=\"1\" stop-color=\"var(--pb-shadow,#58474a)\" stop-opacity=\"0\"/></radialGradient></defs>\n"
        << "<ellipse class=\"pb-contact\" cx=\"24\" cy=\"54\" rx=\"20\" ry=\"3.7\" fill=\"url(#pb-ground-soft)\" "
        << "shape-rendering=\"geometricPrecision\"/>\n"
        << "<svg class=\"pb-window\" width=\"48\" height=\"56\" viewBox=\"0 0 48 56\" overflow=\"hidden\">\n"
        << "<image class=\"pb-atlas\" width=\"" << columns * width << "\" height=\"" << rows * height
        << "\" href=\"data:image/png;base64," << makeAtlas(frames,rows) << "\"/></svg>\n";
    drawLandscape(out);
    out << "</svg>\n";
}
}  // namespace

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "Usage: build-pet [output.svg]\n";
        return 1;
    }
    try {
        const std::string destination = argc == 2 ? argv[1] : "assets/pet-bear-rig.svg";
        std::ofstream out(destination);
        draw(out);
        out.close();
        if (!out) throw std::runtime_error("Cannot write " + destination);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
