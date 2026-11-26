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
    {'o', "#5e443a"}, {'s', "#a97250"}, {'t', "#c98f60"}, {'g', "#e6b477"},
    {'f', "#f3c98f"}, {'h', "#ffdfa4"}, {'l', "#fff0bc"}, {'e', "#b77c63"},
    {'p', "#df9f85"}, {'r', "#edb29b"}, {'m', "#ffe4b6"}, {'w', "#fff0d0"},
    {'n', "#493833"}, {'k', "#352829"}, {'O', "#4f6067"}, {'S', "#92abb3"},
    {'M', "#b3c7ce"}, {'W', "#e1edf0"}, {'F', "#fffcf3"}, {'B', "#527d9b"},
    {'A', "#80a9bd"}, {'C', "#b4ced6"}, {'T', "#476f78"}, {'D', "#648d92"},
    {'I', "#c1ddda"}, {'z', "#69839b"}, {'v', "#8c98b1"}, {'a', "#b5bfd5"},
    {'b', "#d6dbea"}, {'d', "#edeef2"}, {'y', "#e2a052"}, {'j', "#f8c272"},
    {'u', "#925832"}, {'q', "#6b4638"}, {'H', "#70894e"}, {'G', "#acc078"},
    {'R', "#bb5949"}, {'X', "#ab5b50"}, {'V', "#d46e61"}, {'P', "#ed9d82"},
    {'E', "#354b59"}, {'L', "#7ba59f"}, {'Y', "#8fc29e"}, {'Z', "#dad995"},
    {'J', "#c45482"}, {'K', "#ffc1d0"}, {'N', "#9a456c"}, {'Q', "#e589a6"},
    {'0', "#795e4a"}, {'1', "#a77e57"}, {'2', "#c39c70"}, {'3', "#dab78b"},
    {'4', "#ead0a8"}, {'5', "#f4e1bd"}, {'6', "#8da6a2"},
};

template<int Width, int Height>
class PixelGrid {
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

    void write(std::ostream& out, const std::map<char, std::string>& colors = palette) const {
        // Join consecutive pixels by colour. CSS animates the parent SVG
        // groups, so an entire arm only needs one transform at runtime.
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
    // The fabric covers the shoulder pivot in every pose, including a raised paw.
    std::vector<Row> sleeve{{11,15},{10,16},{9,16},{8,16},{7,15},{6,14},{6,13},
                            {5,13},{5,13},{5,13},{5,12},{6,12},{8,11}};
    art.silhouette(40, right ? mirror(paw) : paw, 'o', [](int x, int y, int, int hi) {
        return y >= 44 || x >= hi - 1 ? 't' : 'g';
    });
    art.silhouette(31, right ? mirror(sleeve) : sleeve, 'O', [](int x, int y, int, int hi) {
        if (y >= 40) return 'M';
        return x >= hi - 2 ? 'W' : 'F';
    });
    return art;
}

Pixels torso() {
    Pixels art;
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
    art.stamp(11, 8, {".......lllllll....","....llllllhhhh....","..lllhlhhhh.......",
                       ".llhhhhhh.........",".lhhhh............"});
    art.dots({{12,14},{14,12},{20,10},{13,25},{16,26},{31,27}}, 'h');
    art.span(27, 12, 15, 'g');
    art.span(28, 14, 18, 'g');
    art.span(29, 16, 19, 'g');
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

void beginProp(std::ostream& out, const std::string& name, Point leftCuff, Point rightCuff) {
    beginGroup(out, "pb-prop pb-" + name, false);
    for (bool right : {false, true}) {
        const auto [x, y] = right ? rightCuff : leftCuff;
        // This short sleeve is stretched from its shoulder to the cuff as the prop moves.
        out << "<g class=\"pb-reach pb-reach-" << (right ? 'r' : 'l')
            << "\" style=\"--pb-cuff-x:" << x + 3 << ";--pb-cuff-y:" << y + 2 << "\">\n"
            << "<path fill=\"" << palette.at('O') << "\" d=\"M-2 -2l1 -1h4l6 1v4l-6 1h-4l-1 -1z\"/>\n"
            << "<path fill=\"" << palette.at('F') << "\" d=\"M-1 -2h4l6 1v2l-6 1h-4z\"/>\n"
            << "<path fill=\"" << palette.at('M') << "\" d=\"M-1 1h10v1l-6 1h-4z\"/>\n</g>\n";
    }
    beginGroup(out, "pb-held-object");
}

void drawProps(std::ostream& out) {
    const Point burgerLeft{8,37}, burgerRight{34,37};
    beginProp(out, "burger", burgerLeft, burgerRight);
    layer(out, "pb-burger-full", burger(false));
    layer(out, "pb-burger-bitten", burger(true), false);
    holdingHand(out, false, burgerLeft, {10,39});
    holdingHand(out, true, burgerRight, {32,39});
    out << "</g>\n</g>\n";

    const Point colaLeft{15,40}, colaRight{34,39};
    beginProp(out, "cola", colaLeft, colaRight);
    layer(out, "pb-cola-straw", straw());
    layer(out, "pb-cup", cup());
    holdingHand(out, false, colaLeft, {17,41});
    holdingHand(out, true, colaRight, {32,41});
    out << "</g>\n</g>\n";

    const Point consoleLeft{7,38}, consoleRight{35,38};
    beginProp(out, "console", consoleLeft, consoleRight);
    console().write(out);
    Pixels player;
    player.stamp(21, 37, {".Z","ZZ","ZZ"});
    layer(out, "pb-game-player", player);
    holdingHand(out, false, consoleLeft, {9,40}, true);
    holdingHand(out, true, consoleRight, {33,40}, true);
    out << "</g>\n</g>\n";

    const Point pillowLeft{7,37}, pillowRight{31,38};
    beginProp(out, "pillow", pillowLeft, pillowRight);
    pillow().write(out);
    holdingHand(out, false, pillowLeft, {9,39});
    holdingHand(out, true, pillowRight, {29,40});
    out << "</g>\n</g>\n";
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

void drawProfile(std::ostream& out) {
    Pixels face;
    face.stamp(30, 3, {"..oooo...",".ohhgoo..","ohhfgto..","ogppgsto.","ogprpgto.",
                       ".ogprgto.",".ogggto..","..oooo..."});
    face.stamp(9, 2, {"...oooo....","..ohhhgo...",".ohllhgto..","ohhhffgsto.",
                      "offepppgto.","offeprpgto.","ogfeprpgto.",".ogfgpgto..","..ogggto..."});
    const std::vector<Row> headEdge{
        {20,30},{16,33},{14,35},{12,36},{11,37},{10,38},{9,39},{9,39},{8,39},
        {8,40},{8,40},{8,40},{8,40},{8,40},{8,41},{9,41},{9,41},{10,40},
        {10,40},{11,39},{12,38},{14,37},{16,35},{18,33},{21,31},
    };
    face.silhouette(7, headEdge, 'o', [](int x, int y, int lo, int hi) {
        if (y >= 29 || (y >= 17 && x == hi - 1)) return 't';
        if (y >= 27 || x >= hi - 3) return 'g';
        if (y <= 12 || (x <= lo + 3 && y <= 17)) return 'h';
        return 'f';
    });
    face.stamp(12, 9, {"......lllllll.","...llllllhhhh.","..lllhlhh.....",".llhhhh......."});
    face.dots({{10,23},{12,25},{15,26},{13,26},{18,28},{20,27},{37,25}}, 'g');
    face.span(29, 20, 23, 'g');
    face.stamp(25, 20, {"....mmmmm...","..mmwwwwwmm.",".mmwwwwwwwm.","mmwwwwnnnmm.",
                        "mmwwwwwnwwm.",".mmwwnwnwnm.","..mmwwnnnmm.","....mmmmmm.."});
    face.stamp(14, 21, {"..rrr...",".rprpr..","rprprrr.",".rrr.r.."});
    face.dots({{38,21},{37,22},{38,22}}, 'r');

    face.stamp(16, 13, {"...QQJJJJ...","..Q......J..",".J........J.","J..........J",
                        "J..........J","J..........J",".N........J.","..N......J..","...NJJJJJ..."});
    face.stamp(30, 14, {"..QJJJJ..",".Q.....J.","J.......J","J.......J","J.......J",
                        ".N......J","..N....J.","...NJJJ.."});
    face.span(16, 10, 16, 'J');
    face.span(17, 11, 14, 'N');
    face.span(16, 27, 30, 'Q');
    face.stamp(8, 0, {".NN......N..","NQQNN...NQN.","NKKQQN.NKQQN","NQKQJNNJQQQN",
                       ".NQJQJJQQQN.","..NNNJJJNN..","......N..N.."});

    Pixels eye, closedEye;
    eye.stamp(22, 16, {".nn.","nwnn","nkkk",".kk."});
    eye.stamp(34, 16, {".nn","nwn","nkk",".kk"});
    closedEye.stamp(21, 17, {".kkk.","kk.kk","k...k"});
    closedEye.stamp(33, 17, {".kk.","k..k"});

    Pixels coat, tail;
    const std::vector<Row> coatEdge{{20,30},{18,32},{17,33},{16,34},{15,34},{15,34},
        {15,34},{15,34},{15,34},{15,34},{15,34},{14,34},{14,34},{14,34},{14,34},
        {14,34},{13,34},{13,34},{13,33},{14,32},{15,31},{16,31}};
    coat.silhouette(29, coatEdge, 'O', [](int x, int y, int lo, int hi) {
        if (y >= 48 || x == hi - 1) return 'M';
        return x < lo + 4 || (y >= 43 && x < lo + 7) ? 'W' : 'F';
    });
    coat.stamp(27, 30, {"BBB.","ABBC",".ABC",".BCM","..CM","...M"});
    coat.dots({{27,32},{28,33},{28,34},{29,35},{29,36},{30,37},{30,38},
               {31,39},{31,40},{30,41},{29,40}}, 'T');
    coat.stamp(27, 44, {".MMMM","MF..M",".M.M.","..MM."});
    tail.stamp(9, 39, {"...oo....","..ohgoo..",".ofgfto..","ofgftsto.",
                        "oggtsto..",".ostto...","..ooo...."});

    Pixels farFoot, nearFoot;
    farFoot.stamp(21, 47, {".OOOO.....","OMMMO.....","OMMMO.....",".ooo......",
                           ".ostto....",".osttsoo..",".osttssso.","..ooooooo."});
    nearFoot.stamp(19, 47, {".OOOO.....","OWFMO.....","OMMMO.....",".ooo......",
                            ".ogtto....",".ofggtoo..",".ogttssso.","..ooooooo."});

    Pixels nearArm, farArm;
    nearArm.stamp(18, 31, {"...OOO....","..OFFMO...",".OFFFWMO..",".OFFFWMO..","OFFFFWMO..",
                           "OFFFFWMO..","OFFFWMMO..",".OMMMMMO..",".OMMMMO...","..OOOOO...",
                           "...ogto...","...ofto...","...ogso...","....oo...."});
    farArm.stamp(28, 31, {"..OO......",".OMWO.....",".OMWWO....",".OMWWMO...","..OMWMMO..",
                          "..OMWMMO..","...OMMMO..","...OMMMO..","...OOOO...","....oto...",
                          "....ogso..",".....oo..."});

    out << "<g class=\"pb-profile-shadow\" opacity=\"0\" fill=\"var(--pb-shadow,#6a5849)\">\n"
        << "<path opacity=\".24\" d=\"M13 53h7v-1h16v1h7v1h3v1H34v1H16v-1h-6v-1h3z\"/>\n"
        << "<path opacity=\".34\" d=\"M17 53h14v1h3v1H16v-1h1z\"/>\n</g>\n";
    beginGroup(out, "pb-profile", false);
    beginGroup(out, "pb-profile-body");
    layer(out, "pb-profile-tail", tail);
    layer(out, "pb-profile-far-foot", farFoot);
    layer(out, "pb-profile-far-arm", farArm);
    layer(out, "pb-profile-near-foot", nearFoot);
    layer(out, "pb-profile-coat", coat);
    layer(out, "pb-profile-near-arm", nearArm);
    beginGroup(out, "pb-profile-head");
    face.write(out);
    out << "<path fill=\"var(--pb-light,#ffedba)\" opacity=\".53\" "
        << "d=\"M18 9h6v1h-6zM15 10h4v1h-4zM14 11h3v1h-3zM13 12h2v1h-2z\"/>\n";
    layer(out, "pb-profile-eye", eye);
    layer(out, "pb-profile-smile", closedEye, false);
    out << "</g>\n</g>\n</g>\n";
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

void draw(std::ostream& out) {
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" class=\"pb-rig\" width=\"48\" height=\"56\" "
        << "viewBox=\"0 0 48 56\" overflow=\"visible\" shape-rendering=\"crispEdges\" aria-hidden=\"true\">\n"
        << "<title>戴粉色眼镜的小熊熊女医生</title>\n"
        << "<desc>戴粉色眼镜的像素女医生，带连动衣袖、侧面步态和四季散步场景。</desc>\n"
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
    drawProfile(out);
    drawLandscape(out);
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
