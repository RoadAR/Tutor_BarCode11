//
//  bar_code_recognizer.hpp
//  BarCode11
//
//  Created by Alexander Graschenkov on 19.04.2021.
//

#ifndef bar_code_hpp
#define bar_code_hpp

#include <stdio.h>
#include <string>
#include <opencv2/core.hpp>

#include "decoder.hpp"


class BarCode {
public:
    struct Recognition {
        std::string name;
        std::string value;
        cv::RotatedRect location;
        
        bool empty() const { return name.empty() && value.empty(); }
    };
    
    BarCode(const std::vector<std::shared_ptr<CodeDecoder>> &decoders);
    
    Recognition detectAndRecognize(cv::Mat &image);
    
private:
    std::vector<std::shared_ptr<CodeDecoder>> decoders_;
    cv::Mat gray_;
};

#endif /* bar_code_recognizer_hpp */
