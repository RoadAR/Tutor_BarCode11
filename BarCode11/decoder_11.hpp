//
//  decoder_11.hpp
//  BarCode11
//
//  Created by Alexander Graschenkov on 04.04.2021.
//

#ifndef decoder_11_hpp
#define decoder_11_hpp

#include <stdio.h>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>

struct Code11Symbol {
    Code11Symbol(const std::string &symbol, const std::array<bool, 5> &widePattern);
    std::string symbol;
    std::array<bool, 5> widePattern;
    
    float match(const std::vector<int> &values);
};


class Code11Decoder {
public:
    
    enum Control {
        Raw = -1,
        ControlZero = 0,
        ControlOne = 1,
        ControlTwo = 2,
    } ;
    
    Code11Decoder();
    std::string decode(const std::vector<int> &values, Control control = ControlTwo);
private:
    std::vector<Code11Symbol> symbols;
    std::unordered_map<std::string, int> symbolMap;
    
    bool checkControlSymbol(const std::string &str, int controlNum);
};


#endif /* decoder_11_hpp */
