// Rebuild from the repository root:
// g++ -std=c++17 -O2 _qi-blog/build-pet.cpp -o /tmp/build-pet && /tmp/build-pet
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {
using Point = std::pair<int, int>;
using Row = std::pair<int, int>;  // Inclusive x coordinates of a silhouette on one scanline.
constexpr int width = 48;
constexpr int height = 56;

// S — cool coat shadow, J — rose frames, T — stethoscope. The small colour
// vocabulary keeps edges distinct even on the single-pixel mobile badge.
const std::map<char, std::string> palette = {
    {'o', "#654939"}, {'s', "#b47e50"}, {'t', "#d39b61"}, {'g', "#ecc181"},
    {'f', "#f6cf90"}, {'h', "#ffdfa4"}, {'l', "#ffedba"}, {'e', "#b77c63"},
    {'p', "#df9f85"}, {'r', "#edb29b"}, {'m', "#ffe4b6"}, {'w', "#fff0d0"},
    {'n', "#493833"}, {'k', "#3a302f"}, {'O', "#596367"}, {'S', "#9eacaf"},
    {'M', "#bfcccf"}, {'W', "#eaf0ec"}, {'F', "#fffcf3"}, {'B', "#527d9b"},
    {'A', "#80a9bd"}, {'C', "#b4ced6"}, {'T', "#476f78"}, {'D', "#648d92"},
    {'I', "#c1ddda"}, {'z', "#69839b"}, {'v', "#8c98b1"}, {'a', "#b5bfd5"},
    {'b', "#d6dbea"}, {'d', "#edeef2"}, {'y', "#e2a052"}, {'j', "#f8c272"},
    {'u', "#925832"}, {'q', "#6b4638"}, {'H', "#70894e"}, {'G', "#acc078"},
    {'R', "#bb5949"}, {'X', "#ab5b50"}, {'V', "#d46e61"}, {'P', "#ed9d82"},
    {'E', "#354b59"}, {'L', "#7ba59f"}, {'Y', "#8fc29e"}, {'Z', "#dad995"},
    {'J', "#c46187"}, {'K', "#f4a7bd"}, {'N', "#a74b73"}, {'Q', "#de849e"},
    {'0', "#795e4a"}, {'1', "#a77e57"}, {'2', "#c39c70"}, {'3', "#dab78b"},
    {'4', "#ead0a8"}, {'5', "#f4e1bd"}, {'6', "#8da6a2"},
};

class Pixels {
public:
    void dot(int x, int y, char ink) { pixels.at(y).at(x) = ink; }

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

    void write(std::ostream& out) const {
        // Join consecutive pixels by colour. CSS animates the parent SVG
        // groups, so an entire arm only needs one transform at runtime.
        std::map<char, std::ostringstream> paths;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width;) {
                const int start = x;
                const char ink = pixels[y][x++];
                while (x < width && pixels[y][x] == ink) ++x;
                if (ink) paths[ink] << "M" << start << " " << y << "h" << x - start
                                    << "v1h-" << x - start << "z";
            }
        }
        for (const auto& [ink, path] : paths)
            out << "<path fill=\"" << palette.at(ink) << "\" d=\"" << path.str() << "\"/>\n";
    }

private:
    std::array<std::array<char, width>, height> pixels{};
};

std::vector<Row> mirror(std::vector<Row> rows) {
    for (auto& [left, right] : rows) {
        int newLeft = width - 1 - right;
        right = width - 1 - left;
        left = newLeft;
    }
    return rows;
}

Pixels leg(bool right) {
    Pixels art;
    std::vector<Row> shoe{{15,20},{14,20},{13,21},{12,21},{11,21},{11,21},{12,20}};
    art.silhouette(48, right ? mirror(shoe) : shoe, 'o',
        [](int x, int y, int left, int rightEdge) {
            if (y >= 53 || x == rightEdge - 1) return 's';
            return x == left + 1 ? 'g' : 't';
        });
    return art;
}

Pixels arm(bool right) {
    Pixels art;
    std::vector<Row> paw{{8,12},{6,13},{5,13},{5,13},{5,12},{6,11},{7,10}};
    std::vector<Row> sleeve{{10,12},{8,13},{7,13},{6,13},{6,13},{5,13},{5,12},{6,12},{8,11}};
    art.silhouette(40, right ? mirror(paw) : paw, 'o', [](int x, int y, int, int hi) {
        return y >= 44 || x >= hi - 1 ? 't' : 'g';
    });
    art.silhouette(35, right ? mirror(sleeve) : sleeve, 'O', [](int x, int y, int, int hi) {
        if (y >= 40) return 'M';
        return x >= hi - 2 ? 'W' : 'F';
    });
    return art;
}

Pixels torso() {
    Pixels art;
    // Fixed upper sleeves overlap the moving forearms at the elbows. This
    // leaves a white sleeve behind the raised paw even during a large turn.
    std::vector<Row> shoulder{{14,16},{12,16},{11,16},{10,15},{9,15},{8,15},{8,14},{9,13},{10,12}};
    for (bool right : {false, true})
        art.silhouette(30, right ? mirror(shoulder) : shoulder, 'O',
            [](int x, int, int, int hi) { return x >= hi - 2 ? 'W' : 'F'; });

    std::vector<Row> coat{
        {20,27},{16,31},{14,33},{13,34},{12,35},{12,35},{12,35},{12,35},{12,35},{12,35},
        {12,35},{11,36},{11,36},{11,36},{11,36},{11,36},{11,36},{11,36},{11,36},{10,37},
        {10,37},{11,36},{12,35},
    };
    art.silhouette(28, coat, 'O', [](int x, int y, int lo, int hi) {
        if (x == hi - 1 || y == 49) return 'M';
        return x >= hi - 3 || x == lo + 1 ? 'W' : 'F';
    });
    art.stamp(11, 46, {"..WW..................W..",".W..................WW..",".W........WW........WW.."});
    art.span(49, 13, 23, 'W');
    art.span(49, 25, 34, 'M');
    art.dots({{24,49},{24,48}}, 'S');

    art.stamp(16, 29, {
        "...BBBBBBBBBB...", "..CABBBBBBBBBMC.", "..C.AAABBBBBA.C.", "..WC.AABBBAA.CW.",
        "...WC.ABBBA.CW..", "....MC.ABA.CM...", "...WWMC.B.CMWW..", ".....WMCBCM.....",
        "......WMCW......", ".......MW.......",
    });
    art.dots({{16,31},{15,32},{15,33},{14,34},{14,35},{13,36},{13,37},{13,38},
              {14,39},{14,40},{15,41},{16,41},{17,42},{18,42},{19,42},{20,41},
              {21,40},{21,39},{22,38},{22,37},{22,36}}, 'T');
    art.dots({{14,36},{14,37},{15,39},{16,40},{18,41}}, 'I');
    art.dots({{29,31},{30,32},{31,32},{31,33},{32,34},{32,35},{33,36},{33,37},{32,38},{32,39}}, 'T');
    art.stamp(30, 38, {"..T..",".TTT.","TIDDT","TIWDT",".TDT.","..T.."});
    art.stamp(27, 44, {".....M","WMMMMS","W....M",".W..M.","..MM.."});
    art.dots({{24,41},{24,45}}, 'S');
    return art;
}

Pixels head() {
    Pixels art;
    std::vector<Row> ear{{8,12},{6,14},{5,15},{5,15},{4,16},{4,16},{4,16},{5,15},{5,15},{6,14},{8,12}};
    for (bool right : {false, true})
        art.silhouette(2, right ? mirror(ear) : ear, 'o', [](int x, int y, int, int hi) {
            if (y <= 4) return 'h';
            return y >= 9 || x == hi - 1 ? 't' : 'g';
        });
    for (int x : {7,34})
        art.stamp(x, 5, {"..eee..",".epppe.","eprrrpe","eprrrpe",".epppe.","..eee.."});

    std::vector<Row> face{
        {19,28},{16,31},{13,34},{11,36},{10,37},{9,38},{8,39},{7,40},{7,40},{6,41},{6,41},
        {6,41},{6,41},{6,41},{6,41},{6,41},{6,41},{6,41},{7,40},{7,40},{8,39},{9,38},
        {10,37},{12,35},{14,33},{17,30},
    };
    art.silhouette(6, face, 'o', [](int x, int y, int lo, int hi) {
        if (y >= 28 || (y >= 17 && x >= hi - 2)) return 't';
        if (y >= 26 || x >= hi - 4) return 'g';
        if ((y <= 10 && x < hi - 4) || (x <= lo + 2 && y <= 17)) return 'h';
        return 'f';
    });
    art.span(8, 18, 22, 'l');
    art.span(9, 15, 19, 'l');
    art.span(10, 13, 15, 'l');
    art.span(27, 12, 16, 'g');
    art.span(28, 14, 18, 'g');
    art.stamp(17, 21, {"....mmmmm.....","..mmwwwwwmm...",".mmwwwwwwwmm..","mmwwwwwwwwwmm.",
                       "mmwwwwwwwwwmm.",".mmwwwwwwwmm..","..mmmmmmmmm...","....mmmmm....."});
    art.span(22, 10, 13, 'r');
    art.span(22, 34, 37, 'r');
    art.stamp(21, 22, {".nnnn.","nnnnnn",".nnnn.","..nn.."});
    return art;
}

Pixels glasses() {
    Pixels art;
    for (int x : {9,26}) {
        art.stamp(x, 14, {"...JJJJJJJ...","..J.......J..",".J.........J.","J...........J",
                          "J...........J","J...........J",".J.........J.",".N.........J.",
                          "..N.......J..","...NJJJJJJ..."});
        art.span(14, x + 4, x + 6, 'Q');
        art.dot(x + 2, 15, 'Q');
        art.dot(x + 2, 16, 'K');
    }
    art.span(16, 6, 9, 'J');
    art.span(17, 7, 8, 'N');
    art.span(16, 38, 41, 'J');
    art.span(17, 39, 40, 'N');
    art.dots({{21,17},{26,17}}, 'N');
    art.dots({{22,17},{25,17}}, 'J');
    art.span(16, 23, 24, 'Q');
    return art;
}

Pixels ribbon() {
    Pixels art;
    art.stamp(5, 1, {".NN......N..","NQQNN...NQN.","NKKQQN.NKQQN","NQKQJNNJQQQN",
                     ".NQJQJJQQQN.","..NNNJJJNN..","......N..N.."});
    return art;
}

Pixels eyes(const std::string& name) {
    Pixels art;
    if (name == "open") {
        for (int x : {14,30}) art.stamp(x, 17, {".nn","nwn","nkk",".kk"});
    } else if (name == "happy") {
        for (int x : {13,29}) art.stamp(x, 17, {".kkk.","kk.kk","k...k"});
        for (int x : {9,34}) art.stamp(x, 21, {".rrr.","rprpr",".rrr."});
    } else if (name == "wide") {
        art.stamp(13, 12, {".oo..","o...."});
        art.stamp(29, 12, {"..oo.","....o"});
        for (int x : {14,29}) art.stamp(x, 17, {".nn.","nwkn","nkkk",".nn."});
    } else if (name == "blink") {
        for (int x : {13,29}) art.stamp(x, 19, {"kkkkk",".nnn."});
    } else if (name == "sleep") {
        for (int x : {13,29}) art.stamp(x, 19, {"k...k",".nnn."});
    } else if (name == "focus") {
        art.stamp(12, 16, {"oo..","..o."});
        art.stamp(30, 16, {"..oo",".o.."});
        for (int x : {15,30}) art.stamp(x, 18, {".nn.","nwn.","nkk.",".kk."});
    }
    return art;
}

Pixels mouth(const std::string& name) {
    Pixels art;
    if (name == "neutral") art.stamp(19, 25, {"....n.....",".n..n..n..","..nn.nn..."});
    if (name == "open") art.stamp(20, 25, {"..nnnn..",".nwwwwn.",".nnppnn.","..nrrn..","...nn..."});
    if (name == "chew") art.stamp(17, 25, {".t...n.....t.", "t.m.n.nn....t", ".t...nnn...t."});
    if (name == "smile") art.stamp(19, 25, {"....n.....",".n..n..n..","..nnnnn...","...rpr...."});
    if (name == "sip") art.stamp(22, 25, {".nn.","nppn",".nn."});
    if (name == "burp") art.stamp(21, 25, {".nnn.","npppn","nnrpn",".nnn."});
    return art;
}

Pixels cuff(bool right, int left, int top) {
    Pixels art;
    if (right) {
        art.stamp(left, top, {"..OOO.",".OFFMO","OWFFMO",".OWMSO","..OOO."});
    } else {
        art.stamp(left, top, {".OOO..","OMFFO.","OMFFWO","OSMWO.",".OOO.."});
    }
    return art;
}

Pixels grip(bool right, int left, int top) {
    Pixels art;
    if (right) {
        art.stamp(left, top, {"..ooo.","ottggo","ogffho",".tgggo",".oott."});
    } else {
        art.stamp(left, top, {".ooo..","oggtt.","ohffgo","ogggt.",".ttoo."});
    }
    return art;
}

Pixels burger(bool bitten) {
    Pixels art;
    std::vector<Row> bun{{20,26},{17,29},{15,31},{14,32},{13,33},{13,33},{14,32}};
    art.silhouette(33, bun, 'u', [](int x, int y, int, int hi) {
        return y >= 38 || x >= hi - 2 ? 'y' : 'j';
    });
    art.span(35, 19, 20, 'l');
    art.dot(23, 36, 'l');
    art.span(36, 27, 28, 'l');
    art.dot(16, 37, 'l');
    art.stamp(12, 39, {"..HHHHHHHHHHHHHHHHHHH..", ".HGGGHGGHGGGGHGGHGGGGH.",
                       "..HHRRRRHHHHHRRHHHRHH..", "..qqqqqqqqqqqqqqqqqqq..",
                       "..uyyyyyyyyyyyyyyyyyu..", "...ujjjjjjjjjjjjjjju...",
                       "....uuuuuuuuuuuuuuu...."});
    art.span(42, 26, 29, 'j');
    art.span(43, 27, 28, 'j');
    if (bitten) {
        art.span(33, 22, 27, 0);
        art.span(34, 23, 29, 0);
        art.span(35, 24, 29, 0);
        art.span(36, 25, 28, 0);
        art.dots({{22,34},{23,35},{24,36},{25,37},{28,37},{29,36},{30,35}}, 'u');
        art.span(37, 26, 27, 'l');
    }
    return art;
}

Pixels straw() {
    Pixels art;
    art.stamp(24, 31, {"DDDD....","...II...","....DD..",".....ID.","......ID","......DI",".......I"});
    return art;
}

Pixels cup() {
    Pixels art;
    art.stamp(21, 37, {"...OOOOOOOOOO...",".OSFFFFFFFFFSO..","OWFFFFFFFFFFFFO.",".OMMMMMMMMMMMO..",
                       "..XPPVVVVVVVX...","..XPVVVVVVVVX...","..XVVVVVVVVVX...","..XVVVVVVVVVX...",
                       "...XVVVVVVVX....","...XVVVVVVVX....","...XVVVVVVVX....","...XPVVVVVVX....",
                       "....XVVVVVX.....","....XPPPPPX.....",".....XXXXX......"});
    art.stamp(25, 42, {"......w","....ww.","..www..",".ww...."});
    return art;
}

Pixels pillow() {
    Pixels art;
    std::vector<Row> edge{{12,15},{11,18},{11,22},{10,26},{10,30},{9,32},{9,32},{9,32},
                          {8,31},{8,31},{7,31},{7,30},{8,30},{11,30},{15,29},{19,29},
                          {22,30},{26,30},{28,29}};
    art.silhouette(30, edge, 'v', [](int x, int y, int lo, int hi) {
        if (x >= hi - 2 || y >= 44) return 'a';
        return x <= lo + 1 ? 'b' : 'd';
    });
    art.stamp(10, 32, {"..b........","...bb......","...b......."});
    art.stamp(23, 42, {".....b","....b.","...ba.","....a."});
    return art;
}

Pixels console() {
    Pixels art;
    std::vector<Row> shell{{13,34},{11,36},{10,37},{10,37},{10,37},{10,37},
                           {10,37},{10,37},{10,37},{11,36},{13,34}};
    art.silhouette(34, shell, 'E', [](int x, int y, int, int hi) {
        if (y >= 42 || x >= hi - 1) return 'D';
        return y == 35 ? 'I' : 'L';
    });
    for (int y = 35; y <= 42; ++y) art.span(y, 17, 30, 'E');
    for (int y = 36; y <= 41; ++y) art.span(y, 18, 29, 'T');
    art.span(41, 18, 29, 'Y');
    art.dot(28, 37, 'I');
    art.stamp(12, 37, {".E.","EEE",".E."});
    art.dot(33, 39, 'j');
    art.dot(35, 37, 'P');
    return art;
}

Pixels parcelBack() {
    Pixels art;
    std::vector<Row> rim{{11,36},{9,38},{8,39},{7,40},{6,41},{6,41},{7,40}};
    art.silhouette(33, rim, '0', [](int, int y, int, int) { return y < 36 ? '2' : '1'; });
    art.span(34, 12, 35, '3');
    art.span(36, 11, 36, '0');
    return art;
}

Pixels parcelFront() {
    Pixels art;
    std::vector<Row> edge{{6,41},{6,41},{6,41},{6,41},{6,41},{6,41},{6,41},{6,41},
                          {6,41},{6,41},{7,40},{7,40},{7,40},{7,40},{8,39},{8,39},{9,38}};
    art.silhouette(39, edge, '0', [](int x, int y, int, int hi) {
        if (x >= hi - 2 || y >= 53) return '1';
        return x >= hi - 4 ? '2' : '3';
    });
    art.span(40, 8, 39, '4');
    for (int y = 40; y < 54; ++y) art.span(y, 23, 25, y == 53 ? '2' : '4');
    art.span(41, 23, 25, '5');
    art.stamp(11, 44, {"44444444","45555554","45Q5Q554","45QQQ554","455Q5554","44444444"});
    art.span(50, 31, 34, '1');
    art.span(51, 31, 35, '1');
    return art;
}

Pixels parcelFlap(bool right) {
    Pixels art;
    std::vector<Row> shape{{10,20},{9,21},{8,22},{7,23},{6,23},{7,23}};
    art.silhouette(35, right ? mirror(shape) : shape, '0', [](int, int y, int, int) {
        return y >= 38 ? '3' : '4';
    });
    art.span(36, right ? 28 : 11, right ? 36 : 19, '5');
    return art;
}

void beginGroup(std::ostream& out, const std::string& cssClass, bool visible = true) {
    out << "<g class=\"" << cssClass << "\"" << (visible ? "" : " opacity=\"0\"") << ">\n";
}

void layer(std::ostream& out, const std::string& cssClass, const Pixels& pixels, bool visible = true) {
    beginGroup(out, cssClass, visible);
    pixels.write(out);
    out << "</g>\n";
}

void holdingHand(std::ostream& out, bool right, Point sleeve, Point palm, bool gaming = false) {
    beginGroup(out, right ? "pb-grip-r" : "pb-grip-l");
    cuff(right, sleeve.first, sleeve.second).write(out);
    if (gaming) beginGroup(out, right ? "pb-thumb-r" : "pb-thumb-l");
    grip(right, palm.first, palm.second).write(out);
    if (gaming) out << "</g>\n";
    out << "</g>\n";
}

void drawProps(std::ostream& out) {
    beginGroup(out, "pb-prop pb-burger", false);
    layer(out, "pb-burger-full", burger(false));
    layer(out, "pb-burger-bitten", burger(true), false);
    holdingHand(out, false, {8,37}, {10,39});
    holdingHand(out, true, {34,37}, {32,39});
    out << "</g>\n";

    beginGroup(out, "pb-prop pb-cola", false);
    layer(out, "pb-cola-straw", straw());
    layer(out, "pb-cup", cup());
    Pixels bentSleeve;
    bentSleeve.stamp(9, 38, {"...MWFF......","..MWFFFFM....",".MFFFFFFFM...","..MWFFFFFM...",
                             "....MMMMM...."});
    bentSleeve.write(out);
    holdingHand(out, false, {15,40}, {17,41});
    holdingHand(out, true, {34,39}, {32,41});
    out << "</g>\n";

    beginGroup(out, "pb-prop pb-console", false);
    console().write(out);
    Pixels player;
    player.stamp(21, 37, {".Z","ZZ","ZZ"});
    layer(out, "pb-game-player", player);
    holdingHand(out, false, {7,38}, {9,40}, true);
    holdingHand(out, true, {35,38}, {33,40}, true);
    out << "</g>\n";

    beginGroup(out, "pb-prop pb-pillow", false);
    pillow().write(out);
    holdingHand(out, false, {7,37}, {9,39});
    holdingHand(out, true, {31,38}, {29,40});
    out << "</g>\n";
}

void drawEffects(std::ostream& out) {
    Pixels crumbs;
    crumbs.stamp(28, 27, {".j...","y...j","...u."});
    layer(out, "pb-crumbs", crumbs, false);

    Pixels puff;
    puff.stamp(28, 21, {".....DD....","...DIWWD...","..DIWWWWDD.",".DIWWWWWWID",
                        "..DIWWWWIID","...DDIIDD..","......D...."});
    layer(out, "pb-burp-puff", puff, false);

    beginGroup(out, "pb-sleep-z", false);
    Pixels nearZ;
    nearZ.stamp(40, 9, {"zzz",".z.","zzz"});
    layer(out, "pb-z-near", nearZ);
    Pixels farZ;
    farZ.stamp(43, 2, {"zzzz","...z","..z.",".z..","zzzz"});
    layer(out, "pb-z-far", farZ);
    out << "</g>\n";
}

void draw(std::ostream& out) {
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" class=\"pb-rig\" width=\"48\" height=\"56\" "
        << "viewBox=\"0 0 48 56\" overflow=\"visible\" shape-rendering=\"crispEdges\" aria-hidden=\"true\">\n"
        << "<title>戴粉色眼镜的小熊熊女医生</title>\n"
        << "<desc>可独立活动的头部、手臂、脚、餐点、掌机、睡枕与纸箱。所有铜色、玫瑰粉与青色细节均按像素绘制。</desc>\n"
        << "<defs><clipPath id=\"pb-ground-clip\" clipPathUnits=\"userSpaceOnUse\"><path d=\"M-48 -56h144v112H-48z\"/></clipPath></defs>\n";
    layer(out, "pb-box-back", parcelBack(), false);
    out << "<g clip-path=\"url(#pb-ground-clip)\">\n";
    beginGroup(out, "pb-body");
    layer(out, "pb-leg-l", leg(false));
    layer(out, "pb-leg-r", leg(true));
    layer(out, "pb-torso", torso());

    beginGroup(out, "pb-head");
    head().write(out);
    for (const std::string name : {"open","blink","happy","wide","sleep","focus"})
        layer(out, "pb-eye pb-eye-" + name, eyes(name), name == "open");
    for (const std::string name : {"neutral","open","chew","smile","sip","burp"})
        layer(out, "pb-mouth pb-mouth-" + name, mouth(name), name == "neutral");
    layer(out, "pb-glasses", glasses());
    layer(out, "pb-ribbon", ribbon());
    out << "</g>\n";

    drawProps(out);
    layer(out, "pb-arm-l", arm(false));
    layer(out, "pb-arm-r", arm(true));
    drawEffects(out);
    out << "</g>\n</g>\n";

    layer(out, "pb-box-front", parcelFront(), false);
    layer(out, "pb-box-flap-l", parcelFlap(false), false);
    layer(out, "pb-box-flap-r", parcelFlap(true), false);
    out << "</svg>\n";
}
}  // namespace

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "Usage: build-pet [output.svg]\n";
        return 1;
    }
    const std::string destination = argc == 2 ? argv[1] : "assets/pet-bear-rig.svg";
    std::ofstream out(destination);
    draw(out);
    out.close();
    if (!out) {
        std::cerr << "Cannot write " << destination << '\n';
        return 1;
    }
}
