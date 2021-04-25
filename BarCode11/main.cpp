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
#include <filesystem>
namespace fs = std::filesystem;

using namespace std;
using namespace cv;

string getImagesFolderPath() {
    string result = "../";
    string imgDirName = "test_images";
    
    for (int i = 0; i < 2; i++) {
        bool dirFound = false;
        for (const auto & entry : fs::directory_iterator(result)) {
            if (entry.path().filename().string() == imgDirName) {
                dirFound = true;
                result += imgDirName + "/";
                return result;
            }
        }
        result += "../";
    }
    return "";
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    string imgRootPath = getImagesFolderPath();
    if (imgRootPath.empty()) {
        cout << "Error: Can't find images folder" << endl;
        return 0;
    }
    vector<string> imageNames = {
        "code_11_dashes.jpeg",
        "test1.jpeg",
        "test2.jpeg"};
    for (string name : imageNames) {
        string path = imgRootPath + name;
        cout << "Reading: " << path << endl;
        Mat img = imread(path, IMREAD_GRAYSCALE);
        imshow("img", img);
        
        BarCode barCode({make_shared<Code11Decoder>(Code11Decoder::ControlZero)});
        BarCode::Recognition recog = barCode.detectAndRecognize(img);
        
        cout << "Recognition: " << recog.value << endl;
        waitKey();
    }
    
    
    
    return 0;
}
