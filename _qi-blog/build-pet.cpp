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
    {'o', "#8c5a43"}, {'s', "#ad704f"}, {'t', "#cc8859"}, {'g', "#e7a970"},
    {'f', "#efbb80"}, {'h', "#f8ca92"}, {'l', "#ffe3b3"}, {'e', "#9f5b4f"},
    {'p', "#e59681"}, {'r', "#f4aa89"}, {'m', "#f5c38b"}, {'w', "#fff0cf"},
    {'n', "#793f39"}, {'k', "#59352b"}, {'O', "#84736a"}, {'S', "#a2aaa6"},
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

enum class Face { calm, blink, smile, bite, chew, sip, surprised, sleep, focus, burp };
enum class Item { none, burger, bittenBurger, cola, console, pillow };

constexpr int floorLine = 54;
constexpr int shoulderLine = 33;
constexpr int hipLine = 46;
constexpr int strideLength = 4;

Pixels portrait(bool side, Face expression) {
    Pixels art;
    // The ears and cheeks carry the round outline, including in the angled view.
    art.stamp(4, 2, {"....ooooo....","..oohhhfoo...",".ohhllhhfto..","ohhhhlhhftto.",
                      "ohhhfgrgttto.","ohhfgrprgtso.","ohfgprprgtso.","ofgggrgttso..",
                      ".ofgttttso...","..oottsoo....","....ooo......"});
    art.stamp(side ? 31 : 32, 2, {"....ooooo...","..oohhhfoo..",".ohhlhhftto.",
          "ohhhlhftttso","ohhfgrgtttso","ohfgrprgttso","ofgprgtttso.",
          "oggggtttsoo.",".ogtttsoo...","..ooooo....."});
    const std::vector<Row> face = side
        ? std::vector<Row>{{19,29},{16,31},{14,33},{12,35},{11,36},{10,38},{9,39},
            {8,40},{8,40},{7,41},{7,41},{7,41},{7,42},{7,42},{8,42},{8,42},
            {9,41},{9,41},{10,40},{11,39},{12,38},{14,37},{16,35},{18,33},{21,30},{23,28}}
        : std::vector<Row>{{19,28},{16,31},{14,33},{12,35},{11,36},{10,37},{9,38},
            {8,39},{7,40},{7,40},{6,41},{6,41},{6,41},{6,41},{6,41},{7,40},
            {7,40},{8,39},{9,38},{10,37},{11,36},{13,34},{15,32},{17,30},{19,28},{21,26}};
    art.silhouette(7, face, 'o', [side](int x, int y, int lo, int hi) {
        if (y >= 29 || x == hi - 1) return 't';
        if (y >= 26 || x >= hi - 3 || (side && x <= lo + 2 && y > 17)) return 'g';
        return y <= 14 || x <= lo + 3 ? 'h' : 'f';
    });
    art.stamp(12, 10, {".....lllllllll.......","...llllllhhhhhhh......",
                       "..lllhhlhhhhhh.........",".lllhhhhhhh............"});
    art.dots({{10,16},{11,14},{14,12},{15,15},{18,12},{27,11},{30,14}}, 'l');
    art.stamp(8, 21, {"..rrrr..",".rprrrr.","rprrrrg.","rprrrg..",".rrrg..."});
    art.stamp(side ? 35 : 33, 21, {".rrr..","rprrr.","rprrg.",".rrg.."});

    const int nose = side ? 29 : 23;
    const int nearLens = side ? 15 : 10;
    const int farLens = side ? 30 : 26;
    art.stamp(nose - 6, 22, {"...mmmmmm....","..mmwwwwmm...",".mmwwwwwwmm..",
                           "mmwwwwwwwwmm.",".mmwwwwwwmm..","..mwwwwmm....","...mmmm......"});
    art.stamp(nose - 2, 23, {".nnn.","nprpn",".nnn."});
    switch (expression) {
        case Face::bite:
        case Face::surprised:
        case Face::burp:
            art.stamp(nose - 2, 27, {".nnn.","nkppn",".nnn."}); break;
        case Face::chew:
            art.stamp(nose - 3, 27, {"m..nn.m","..nrrn.","...nn.."}); break;
        case Face::sip:
            art.stamp(nose - 1, 27, {".n.","npn",".n."}); break;
        case Face::sleep:
            art.stamp(nose - 1, 27, {"nnn",".n."}); break;
        default:
            art.stamp(nose - 4, 26, {"n...n...n",".nnnnnnn.","..nrprn..","...nnn..."});
            break;
    }

    art.stamp(nearLens, 14, {"..QQQJJJ..",".Q......J.","J........J","J........J","J........J",
                            "J........J","J........J",".N......J.","..NNJJJJ.."});
    art.stamp(farLens, 14, side
        ? std::initializer_list<std::string>{".QQJJJ..","Q.....J.","J......J","J......J","J......J",
            "J......J",".N....J.","..NJJJ.."}
        : std::initializer_list<std::string>{"..QQQJJJ..",".Q......J.","J........J","J........J","J........J",
            "J........J","J........J",".N......J.","..NNJJJJ.."});
    art.span(17, nearLens - 3, nearLens + 1, 'N');
    art.dots({{nearLens + 9,18},{farLens,18}}, 'N');
    art.span(17, nearLens + 10, farLens - 1, 'Q');
    if (!side) art.span(17, farLens + 9, farLens + 12, 'J');

    const int eye[]{nearLens + 2, farLens + (side ? 1 : 2)};
    for (const int x : eye) {
        if (expression == Face::blink || expression == Face::sleep) art.stamp(x, 19, {"n...n",".nnn."});
        else if (expression == Face::smile || expression == Face::chew)
            art.stamp(x, 17, {"..k..",".k.k.","k...k"});
        else {
            if (side && x == eye[1]) art.stamp(x, 16, {"..www.",".wnkkw","wnlkkw","wnkkkw",
                                                       "wnkkkw",".wkkw.","..ww.."});
            else art.stamp(x - 1, 16, {"..www..",".wnkkw.","wnllkkw","wnlkkkw",
                                       "wnkkkkw",".wkkkw.","..www.."});
            if (expression == Face::focus) art.dot(x + 3, 18, 'k');
            if (expression == Face::surprised || expression == Face::burp) art.dot(x + 3, 19, 'w');
        }
    }

    // A small berry-coloured ribbon sits on the ear, above the glasses.
    art.stamp(7, 7, {".NN......N..","NQQN....NJN.","NKQQN..NQJN.","NQQQJNNQQJN.",
                      ".NNJJQJJJNN.","...NNJN.NN..","....NJN..N..","....NN...N.."});
    return art;
}

Pixels coat(bool side) {
    Pixels art;
    const std::vector<Row> belly = side
        ? std::vector<Row>{{20,28},{18,31},{16,33},{15,34},{14,35},{14,35},{13,35},{13,35},
            {13,35},{13,35},{13,35},{14,35},{14,34},{15,34},{16,33},{17,32},{18,31},{20,29},{23,27}}
        : std::vector<Row>{{18,29},{15,32},{14,33},{13,34},{12,35},{12,35},{12,35},{12,35},
            {12,35},{12,35},{12,35},{13,34},{13,34},{14,33},{15,32},{16,31},{17,30},{19,28},{21,26}};
    art.silhouette(31, belly, 'o', [](int x, int y, int lo, int hi) {
        if (y >= 47 || x >= hi - 1) return 't';
        return x <= lo + 3 || y > 44 ? 'g' : 'h';
    });
    const std::vector<Row> jacket = side
        ? std::vector<Row>{{20,29},{17,32},{15,33},{14,34},{13,34},{13,34},{13,34},
            {13,34},{13,34},{13,34},{13,34},{14,34},{14,33},{15,32},{17,31},{20,29}}
        : std::vector<Row>{{18,29},{15,32},{14,33},{13,34},{12,35},{12,35},{12,35},
            {12,35},{12,35},{12,35},{12,35},{13,34},{13,34},{14,33},{16,32},{19,29}};
    art.silhouette(31, jacket, 'O', [](int x, int y, int lo, int hi) {
        if (y >= 45 || x >= hi - 1) return 'M';
        return x <= lo + 2 || y >= 43 || x >= hi - 3 ? 'W' : 'F';
    });
    if (side) {
        art.stamp(25, 33, {".WCB..","..MBB.","..MFhO","...MhO","...MhO","...MhO","...Mgo","...Mgo",
                          "...Mgo","..MMho"});
        art.dots({{22,33},{23,34},{24,35},{24,36},{24,37},{25,38},{26,38},{27,37}}, 'T');
        art.stamp(24, 38, {".TT.","TZCT",".TT."});
        art.stamp(18, 41, {".WWW.","FWMM.",".MM.."});
    } else {
        art.stamp(17, 33, {"..FFWCWFW....","...MFGFWM....","...MFhfWM....","...WMhhMW....",
                           "....MhhhM....","....MhhhM....","....MhhhM....","....MhhhM....",
                           "...WMhhgMW...","...MghgggM...","...ogggggo...","....oggoo...."});
        art.dots({{17,33},{17,34},{17,35},{17,36},{18,37},{19,38},{20,38},{21,37},{21,36}}, 'T');
        art.dots({{28,33},{29,34},{29,35},{29,36},{29,37}}, 'T');
        art.stamp(27, 37, {".TT.","TZCT",".TT."});
        art.stamp(14, 40, {"..WW..",".FWM..",".WWMO.","..MM.."});
        art.stamp(28, 41, {".WM..",".FWMO",".MMO.","..M.."});
    }
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
    // Each arm has two six-pixel bones. Only the bend and wrist position vary.
    const double dx = hand.first - shoulder.first, dy = hand.second - shoulder.second;
    const double length = std::max(1.0, std::hypot(dx, dy));
    if (length > 12.25) throw std::runtime_error("Wrist outside arm reach");
    const double arc = bend * std::sqrt(std::max(0.0, 36 - length * length / 4)) / length;
    return {int(std::lround((shoulder.first + hand.first) / 2.0 - dy * arc)),
            int(std::lround((shoulder.second + hand.second) / 2.0 + dx * arc))};
}

void sleeve(Pixels& art, Point shoulder, Point hand, int bend, bool shaded) {
    const Point joint = elbow(shoulder, hand, bend);
    stroke(art, shoulder, joint, 3, 'O');
    stroke(art, shoulder, joint, 2, shaded ? 'W' : 'F');
    const Point wrist = between(joint, hand, .87);
    stroke(art, joint, wrist, 2, 'o');
    stroke(art, joint, wrist, 1, shaded ? 't' : 'h');
    stroke(art, wrist, hand, 1, shaded ? 'g' : 'h');
}

void paw(Pixels& art, Point hand, bool shaded) {
    art.stamp(hand.first - 3, hand.second - 2, shaded
        ? std::initializer_list<std::string>{"..ooo..",".ogtto.","ogfgsso","oggtsso",".ogtto.","..ooo.."}
        : std::initializer_list<std::string>{"..ooo..",".ohhfo.","ohhfgto","ohffgto",".ogtto.","..ooo.."});
}

void plushFoot(Pixels& art, Point hip, Point foot, bool side, bool shaded) {
    const Point ankle{foot.first, foot.second - 5};
    stroke(art, hip, ankle, 3, 'o');
    stroke(art, hip, ankle, 2, shaded ? 't' : 'h');
    if (side) {
        art.stamp(foot.first - 3, foot.second - 5, shaded
            ? std::initializer_list<std::string>{".oooo....","otggto...","oggtsto..","ogggttso.",".otggtso.","..ooooo.."}
            : std::initializer_list<std::string>{".oooo....","ohhgto...","ohffhto..","ofhhhfto.",".ohhgtto.","..ooooo.."});
    } else {
        art.stamp(foot.first - 4, foot.second - 5, {"..ooooo..",".ohhhgto.","ohhfhfgto",
                                                   "ofhhfggto",".ogggtto.","..ooooo.."});
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
    Point nod{};
    Face face = Face::calm;
    Point leftHand{13, 43};
    Point rightHand{34, 43};
    Point leftFoot{18, floorLine};
    Point rightFoot{29, floorLine};
    Item item = Item::none;
    Point prop{24, 34};
    int rightBend = -1;
};

Pose sidePose() {
    Pose pose;
    pose.side = true;
    pose.leftHand = {29, 41};
    pose.rightHand = {20, 43};
    pose.leftFoot = {25, floorLine};
    pose.rightFoot = {21, floorLine};
    return pose;
}

Pixels actor(const Pose& pose, int moment) {
    Pixels body;
    const Point leftShoulder{pose.side ? 28 : 15, shoulderLine + pose.chest};
    const Point rightShoulder{pose.side ? 20 : 32, shoulderLine + pose.chest};
    const auto handL = plus(pose.leftHand, {0, pose.chest});
    const auto handR = plus(pose.rightHand, {0, pose.chest});
    if (pose.side) {
        Pixels tail;
        tail.stamp(9, 41, {"..oo..",".ohgo.","ofgto.",".tso..","..oo.."});
        body.over(tail, 0, pose.chest);
    }
    plushFoot(body, {pose.side ? 25 : 18, hipLine + pose.chest}, pose.leftFoot, pose.side, pose.side);
    plushFoot(body, {pose.side ? 21 : 29, hipLine + pose.chest}, pose.rightFoot, pose.side, false);
    if (pose.side) {
        sleeve(body, leftShoulder, handL, 1, true);
        paw(body, handL, true);
    }
    body.over(coat(pose.side), 0, pose.chest);
    body.over(portrait(pose.side, pose.face), pose.nod.first, pose.chest + pose.nod.second);
    if (!pose.side) sleeve(body, leftShoulder, handL, 1, false);
    sleeve(body, rightShoulder, handR, pose.rightBend, pose.side);
    body.over(food(pose.item, pose.prop, moment), 0, pose.chest);
    if (!pose.side) paw(body, handL, false);
    paw(body, handR, pose.side);
    Pixels frame;
    frame.over(body, pose.x, pose.lift);
    return frame;
}

void hold(Pose& p, Item item, int x, int y) {
    p.item = item;
    p.prop = {x, y};
    if (item == Item::cola) {
        p.leftHand = {x - 7, y + 2};
        p.rightHand = {x + 4, y + 3};
        p.rightBend = 1;
    } else {
        const int reach = item == Item::console ? 9 : 7;
        const int grip = item == Item::console ? 4 : 5;
        p.leftHand = {x - reach, y + grip};
        p.rightHand = {x + reach, y + grip};
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
    Pose pose;
    if (scene == "walk") {
        pose = sidePose();
        const int stride[] = {2,1,0,-1,-2,-1,0,1};
        const int rise[] = {0,0,0,0,0,1,2,1};
        const int left = (frame + 4) % 8, right = frame % 8;
        pose.leftFoot = {25 + stride[left], floorLine - rise[left]};
        pose.rightFoot = {21 + stride[right], floorLine - rise[right]};
        pose.leftHand.first -= stride[left] / 2;
        pose.rightHand.first -= stride[right];
        pose.chest = frame % 4 == 2 ? -1 : 0;
    } else if (scene == "inspect" || scene == "inspectUp") {
        pose = sidePose();
        const double lean = blend(seconds,.23,1.06) - blend(seconds,1.75,2.65);
        pose.chest = pixel(lean);
        pose.nod = {pixel(2 * lean), pixel((scene == "inspect" ? 3 : -2) * lean)};
        pose.rightHand = between({20,43}, {26,40}, lean);
        pose.face = seconds > .86 && seconds < 2.12 ? Face::smile : Face::calm;
    } else if (scene == "turn") {
        pose.face = Face::blink;
        if (frame == 1) pose.rightFoot.second--;
    } else if (scene == "pat") {
        const double joy = blend(seconds,.05,.42) - blend(seconds,1.82,2.36);
        pose.face = joy > .35 ? Face::smile : Face::calm;
        pose.chest = -pixel(joy * .8);
        pose.nod = {pixel(-2 * joy), pixel(joy)};
        pose.leftHand = between({13,43},{12,41},joy);
        pose.rightHand = between({34,43},{35,41},joy);
    } else if (scene == "feed") {
        if (seconds >= .27 && seconds < 4.55) {
            const double lift = blend(seconds,.62,1.8) - blend(seconds,2.52,3.5);
            const bool bitten = seconds >= 2.12;
            hold(pose, bitten ? Item::bittenBurger : Item::burger, 24, 38 - pixel(12 * lift));
            if (seconds >= 1.69 && !bitten) pose.face = Face::bite;
            if (bitten) pose.face = frame % 3 != 0 ? Face::chew : Face::smile;
            pose.nod.second = seconds > 1.78 && seconds < 2.83 ? 1 : 0;
        } else if (seconds >= 4.85 && seconds < 7.98) {
            const double lift = blend(seconds,5.2,5.95) - blend(seconds,6.84,7.78);
            hold(pose, Item::cola, 29, 39 - pixel(4 * lift));
            if (lift > .86) pose.face = Face::sip;
            if (seconds > 7.18) pose.face = Face::smile;
        } else if (seconds >= 7.98 && seconds < 9.72) {
            pose.face = seconds >= 8.90 && seconds <= 9.36 ? Face::burp : Face::smile;
            pose.chest = pose.face == Face::burp ? -1 : 0;
            pose.leftHand = between({13,43}, {19,43}, blend(seconds,7.9,8.7) - blend(seconds,9.25,9.72));
            pose.rightHand = between({34,43}, {30,43}, blend(seconds,7.9,8.7) - blend(seconds,9.25,9.72));
        }
    } else if (scene == "dance" && frame > 0 && seconds < 3.08) {
        const int step = frame % 8;
        const int side = frame % 16 < 8 ? 1 : -1;
        pose.face = Face::smile;
        pose.x = side * (step < 4 ? step : 8 - step);
        pose.lift = step == 2 || step == 3 ? -2 : 0;
        pose.leftHand = {13 - side, 31 + step / 2};
        pose.rightHand = {35 + side, 28 - step / 2};
        pose.leftFoot.first -= step / 2;
        pose.rightFoot.first += step / 2;
    } else if (scene == "sleep" && seconds > .28 && seconds < 9.55) {
        const double lean = blend(seconds,.32,1.35) - blend(seconds,8.95,9.54);
        hold(pose,Item::pillow,24,39 - pixel(4 * lean));
        pose.nod = {pixel(-2 * lean), pixel(2 * lean)};
        pose.chest = pixel(lean * (.7 + .3 * std::sin(seconds * 3)));
        pose.face = lean > .38 ? Face::sleep : Face::blink;
    } else if (scene == "game" && seconds > .23 && seconds < 7.75) {
        const double raise = blend(seconds,.23,.9) - blend(seconds,7.1,7.75);
        hold(pose,Item::console,24,40 - pixel(4 * raise));
        pose.leftHand.second -= frame % 3 == 0;
        pose.rightHand.second -= frame % 5 == 0;
        pose.nod.second = seconds < 5.84 ? 1 : 0;
        pose.face = seconds > 5.68 ? Face::smile : Face::focus;
        if (seconds > 5.81 && seconds < 6.34) pose.lift = -pixel(4 * std::sin((seconds - 5.81) / .53 * 3.14));
    } else if (scene == "drag") {
        pose.face = Face::surprised;
        pose.lift = -2;
        pose.leftFoot.second -= frame % 3;
        pose.rightFoot.second -= (frame + 1) % 3;
        pose.leftHand = {11, 40 - frame % 3};
        pose.rightHand = {36, 39 + frame % 3};
    } else if (scene == "enter" || scene == "exit") {
        const bool entering = scene == "enter";
        const bool box = entering ? seconds < 1.49 : seconds >= .63;
        const bool shut = entering ? seconds < .22 : seconds > 1.42;
        if (entering) {
            pose.lift = 24 - pixel(5 * blend(seconds,.36,.65) + 24 * blend(seconds,.96,1.36))
                + pixel(5 * blend(seconds,1.36,1.62));
            pose.chest = seconds >= 1.56 && seconds <= 1.66 ? 1 : 0;
            wave(pose, seconds - 1.47);
        } else {
            pose.lift = -pixel(4 * blend(seconds,.6,.88)) + pixel(36 * blend(seconds,.89,1.38));
            pose.chest = seconds >= .52 && seconds <= .65 ? 1 : 0;
            wave(pose, seconds);
        }
        pose.face = seconds > (entering ? 1.3 : .1) ? Face::smile : Face::calm;
        Pixels result;
        if (box) result.over(parcel(shut, false));
        if (!shut || !box) result.over(actor(pose, frame));
        if (box) result.over(parcel(shut, true));
        if (entering && seconds >= 1.57 && seconds < 1.96) dust(result,24,54,pixel((seconds - 1.57) * 10));
        return result;
    } else if (scene == "rest") {
        if (seconds > 4.6 && seconds < 4.86) pose.face = Face::blink;
        pose.chest = seconds > 1.0 && seconds < 1.78 ? -1 : 0;
    }

    Pixels result = actor(pose, frame);
    if (scene == "walk" && frame % 4 < 2) {
        const int x = 15 - frame % 4;
        result.dot(x, 53 - frame % 4, 'j');
        result.dot(x + 2, 54, 'l');
    }
    if (scene == "pat") {
        if (seconds > .25 && seconds < 1.35) heart(result, 39, 11 - pixel(seconds * 5));
        if (seconds > .91 && seconds < 1.84) heart(result, 43, 18 - pixel(seconds * 5));
    }
    if (scene == "feed" && seconds >= 2.10 && seconds < 2.38)
        result.dots({{31,27+(frame%3)},{34,30+(frame%3)},{29,31}},'j');
    if (scene == "feed" && seconds >= 8.93 && seconds < 9.5) {
        const int x = 35 + pixel((seconds - 8.93) * 9), y = 25 - pixel((seconds - 8.93) * 5);
        result.stamp(x, y, {"...DD..",".DIWWD.","DIWFWID",".DIIDD.","...D..."});
    }
    if (scene == "sleep" && pose.face == Face::sleep) {
        const int y = 7 - (frame % 15) / 3;
        result.stamp(38, y, {"CCC","..C",".C.","CCC"});
        result.stamp(43, y + 7, {"CC",".C","CC"});
    }
    if ((scene == "dance" && frame > 3 && frame < 28) ||
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
        {"rest",5600,28,true,{}}, {"walk",640,8,true,{}}, {"turn",180,3,false,{}},
        {"inspect",2700,27,false,{}}, {"inspectUp",2700,27,false,{}},
        {"pat",2400,24,false,{}}, {"feed",10000,100,false,{}}, {"dance",3200,32,false,{}},
        {"sleep",10000,50,false,{}}, {"game",8000,64,false,{}},
        {"enter",2200,22,false,{}}, {"exit",1700,17,false,{}}, {"drag",720,6,true,{}},
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
        << "<title>小熊熊 · 奶油毛绒像素</title>\n"
        << "<desc>圆脸大眼睛的奶油色像素小熊，戴粉色眼镜，穿白色小外套。各动作使用同尺寸像素帧。</desc>\n"
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
