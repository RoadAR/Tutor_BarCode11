//
//  decoder_11.hpp
//  BarCode11
//
//  Created by Alexander Graschenkov on 04.04.2021.
//

#ifndef decoder_11_hpp
#define decoder_11_hpp

#include <stdio.h>
#include <array>
#include <string>
#include <unordered_map>
#include "decoder.hpp"

struct Code11Symbol {
    Code11Symbol(const std::string &symbol, const std::array<bool, 5> &widePattern);
    std::string symbol;
    std::array<bool, 5> widePattern;
    
    float match(const std::vector<int> &values) const;
};


class Code11Decoder: public CodeDecoder {
public:
    enum Control {
        Raw = -1,
        ControlZero = 0,
        ControlOne = 1,
        ControlTwo = 2,
    };
    
    Control control;
    
    Code11Decoder(Control control = ControlTwo);
    ~Code11Decoder() override = default;
    std::string decode(const std::vector<int> &values) const override;
    std::string getName() const override {
        return "Code11";
    }
private:
    std::vector<Code11Symbol> symbols_;
    std::unordered_map<std::string, int> symbolMap_;
    
    std::string rawDecode(const std::vector<int> &values) const;
    bool checkControlSymbol(const std::string &str, int controlNum) const;
};


#endif /* decoder_11_hpp */
