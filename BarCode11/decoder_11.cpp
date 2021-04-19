//
//  decoder_11.cpp
//  BarCode11
//
//  Created by Alexander Graschenkov on 04.04.2021.
//

#include "decoder_11.hpp"

using namespace std;

Code11Symbol::Code11Symbol(const std::string &symbol, const std::array<bool, 5> &widePattern): symbol(symbol), widePattern(widePattern) {
}

float Code11Symbol::match(const std::vector<int> &values) {
    if (values.size() != 5) return 0;
    vector<int> valuesSorted(values.size());
    for (int i = 0; i < values.size(); i++) {
        valuesSorted[i] = abs(values[i]);
    }
    sort(valuesSorted.begin(), valuesSorted.end());
    float minVal = (valuesSorted[0]+valuesSorted[1]+valuesSorted[2]) / 3.0;
    float maxVal = valuesSorted.back();
    
    float score = 0;
    for (int i = 0; i < 5; i++) {
        if (widePattern[i]) {
            score += abs(abs(values[i]) - minVal) / (float)(maxVal - minVal);
        } else {
            score += (maxVal - abs(values[i])) / (float)(maxVal - minVal);
        }
    }
    return score;
}

Code11Decoder::Code11Decoder() {
    symbols = {
        Code11Symbol("0", {0, 0, 0, 0, 1}),
        Code11Symbol("1", {1, 0, 0, 0, 1}),
        Code11Symbol("2", {0, 1, 0, 0, 1}),
        Code11Symbol("3", {1, 1, 0, 0, 0}),
        Code11Symbol("4", {0, 0, 1, 0, 1}),
        Code11Symbol("5", {1, 0, 1, 0, 0}),
        Code11Symbol("6", {0, 1, 1, 0, 0}),
        Code11Symbol("7", {0, 0, 0, 1, 1}),
        Code11Symbol("8", {1, 0, 0, 1, 0}),
        Code11Symbol("9", {1, 0, 0, 0, 0}),
        Code11Symbol("-", {0, 0, 1, 0, 0}),
        Code11Symbol("*", {0, 0, 1, 1, 0}),
    };
    
    for (int i = 0; i < symbols.size(); i++) {
        symbolMap[symbols[i].symbol] = i;
    }
}

std::string Code11Decoder::decode(const std::vector<int> &values, Control control) {
    string result = "";
    for (int i = 0; i < values.size(); i++) {
        if (values[i] >= 0) continue;
        if (i + 5 > values.size()) {
            break;
        }
        vector<int> sub(values.begin() + i, values.begin() + i + 5);
        
        float maxMatch = -1;
        int maxMatchIdx = -1;
        for (int si = 0; si < symbols.size(); si++) {
            float match = symbols[si].match(sub);
            if (match > maxMatch) {
                maxMatchIdx = si;
                maxMatch = match;
            }
        }
        
        if (maxMatchIdx >= 0) {
            result += symbols[maxMatchIdx].symbol;
            i+=5;
        }
    }
    
    
    if (control == Raw) {
        return result;
    }
    
    // final check
    if (result.size() < 2 || result.front() != '*' || result.back() != '*') {
        return "";
    }
    result = result.substr(1, result.size()-2);
    bool isOk = true;
    for (int i = 0; i < result.size(); i++) {
        if (result[i] == '*') isOk = false;
    }
    
    int controlCount = (int)control;
    while (controlCount > 0) {
        isOk &= checkControlSymbol(result, controlCount);
        result = result.substr(0, result.length()-1);
        controlCount--;
    }
    
    return isOk ? result : "";
}

bool Code11Decoder::checkControlSymbol(const std::string &str, int controlNum) {
    int maxWeight = controlNum == 1 ? 10 : 9;
    int weight = 1;
    int sum = 0;
    
    for (int i = (int)str.length()-2; i >= 0; i--) {
        string symb = str.substr(i, 1);
        int val = symbolMap[symb];
        sum += weight * val;
        weight++;
        if (weight > maxWeight) {
            weight = 1;
        }
    }
    int control = symbolMap[str.substr(str.length()-1, 1)];
    sum = sum % 11;
    
    return sum == control;
}
