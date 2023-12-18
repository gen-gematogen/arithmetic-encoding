#include <bitset>
#include <climits>
#include <cmath>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

const static int N = 100;
const static char eof = 0;

std::unordered_map<char, double> to_cum_prob(std::unordered_map<char, double> &prob){
  std::unordered_map<char, double> cum_prob;
  cum_prob[0] = 0;
  for (int c = 1; c <= 127; ++c){
    cum_prob[(char)c] = cum_prob[c - 1] + prob[c - 1];
  }
  return cum_prob;
}

std::pair<std::bitset<N>, int> encode(std::string msg, std::unordered_map<char, double> prob){
  double min = 0, max = 1, w;
  std::bitset<N> bin_repr;
  int k = 0;
  auto cum_prob = to_cum_prob(prob);

  msg += eof;
  int s = 0;

  for (char l: msg){
    w = max - min;
    max = min + w * (l < 127 ? cum_prob[l + 1] : 1);
    min = min + w * cum_prob[l];
 
    while (max < 0.5 || min > 0.5){
      if (max < 0.5){
        min *= 2;
        max *= 2;
        
        bin_repr[k++] = 0;
        for (int i = 0; i < s; ++i){
          bin_repr[k++] = 1;
        }
        
        s = 0;
      } 
      else{
        min = 2 * (min - 0.5);
        max = 2 * (max - 0.5);
        
        bin_repr[k++] = 1;
        for (int i = 0; i < s; ++i){
          bin_repr[k++] = 0;
        }
        
        s = 0;
      }
    }

    while (min > 0.25 && max < 0.75){
      min = 2 * (min - 0.25);
      max = 2 * (max - 0.25);
      s++;
    }
  }

  s++;

  if (min <= 0.25){
    bin_repr[k++] = 0;
    for (int i = 0; i < s; ++i){
      bin_repr[k++] = 1;
    }
  }
  else{
    bin_repr[k++] = 1;
    for (int i = 0; i < s; ++i){
      bin_repr[k++] = 0;
    }
  }

  return {bin_repr, k};
}

std::string decode(std::unordered_map<char, double> prob, std::bitset<N> data, int k){
  double value;

  for (int i = 1; i <= k; ++i){
    value += 1.0 / (1 << i) * data[i - 1];
  }

  auto cum_prob = to_cum_prob(prob);
  std::string ans = "";
  bool fl = true;
  // double low = 0, high = 1;

  // for (int i = 0; i < 127; ++i) {
  //   std::cout << i << " " << (char)i << " " << cum_prob[i] << "\n";
  // }

  std::cout << "value: " << value << "\n";
  
  while (fl){
    // double w = high - low;
    for (int i = 0; i < 127; ++i){
      //if (low + w * cum_prob[i] <= value &&  value < low + w * cum_prob[i + 1]){
      if (cum_prob[i] <= value && value < cum_prob[i + 1]){
        ans += (char) i;
        // high = low + w * cum_prob[i + 1];
        //low = low + w * cum_prob[i];
        value = (value - cum_prob[i]) / prob[i];

        if (i == eof){
          fl = false;
        }
        break;
      }
    }
    // std::cout << value << " "; 
  }
  // std::cout << "\n";

  return ans;
}

int main(){
  std::unordered_map<char, double> test_prob;

  for (char c = 'a'; c <= 'z'; ++c){
    test_prob[c] = 0.9 / ('z' - 'a' + 1);
  }
  // test_prob['a'] = test_prob['b'] = 0.45;
  test_prob[eof] = 0.05;
  test_prob[' '] = 0.05;

  std::string test_msg;

  std::cin >> test_msg;

  auto ans = encode(test_msg, test_prob);

  for (int i = 0; i < ans.second; ++i){
    std::cout << ans.first[i];
  }
  std::cout << "\n";

  auto dec_s = decode(test_prob, ans.first, ans.second);

  std::cout << dec_s << "\n";

  return 0;
}
