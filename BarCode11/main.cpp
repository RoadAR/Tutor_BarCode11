//
//  main.cpp
//  BarCode11
//
//  Created by Alexander Graschenkov on 04.04.2021.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "BarCode/bar_code.hpp"
#include "BarCode/decoder_11.hpp"

using namespace std;
using namespace cv;



int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    Mat img = imread("/Users/alex/Downloads/photo_2021-04-04 22.48.25.jpeg", IMREAD_GRAYSCALE);
    imshow("img", img);
    
    BarCode barCode({make_shared<Code11Decoder>(Code11Decoder::ControlZero)});
    BarCode::Recognition recog = barCode.detectAndRecognize(img);
    
    cout << "Recognition: " << recog.value << endl;
    
    
    waitKey();
    
    return 0;
}
