//
//  decoder.hpp
//  BarCode11
//
//  Created by Alexander Graschenkov on 19.04.2021.
//

#ifndef decoder_hpp
#define decoder_hpp

#include <stdio.h>
#include <vector>

class CodeDecoder {
public:
    virtual ~CodeDecoder() = default;
    
    virtual std::string decode(const std::vector<int> &values) const = 0;
    
    virtual std::string getName() const = 0;
};

#endif /* decoder_hpp */
