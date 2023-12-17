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

std::vector<double> encode(std::string msg, std::unordered_map<char, double> prob){
  double low = 0, high = 1, w;
  auto cum_prob = to_cum_prob(prob);

  for (char l: msg){
    w = high - low;
    high = low + w * (l < 127 ? cum_prob[l + 1] : 1);
    low = low + w * cum_prob[l];
  }

  w = high - low;
  high = low + w * (eof < 127 ? cum_prob[eof + 1] : 1);
  low = low + w * cum_prob[eof];

  return {low, high};
}

std::pair<std::bitset<N>, int> interval_to_binary(double min, double max){
  std::bitset<N> bin_repr;
  double low = 0, high = 1;
  int k = 0;

  while (low < min || high > max){
    double m = (high + low) / 2;
    if (m > max){
      high = m;
      bin_repr[k] = 0;
    } 
    else if (m < min){
      low = m;
      bin_repr[k] = 1;
    }
    else{
      if (m - min < max - m){
        low = m;
        bin_repr[k] = 1;
      }
      else{
        high = m;
        bin_repr[k] = 0;
      }
    }

    k++;
  }

  return {bin_repr, k};
}

std::string decode(std::unordered_map<char, double> prob, std::bitset<N> data, int k){
  double value;

  for (int i = 1; i <= k; ++i){
    value += 1.0 / (1 << i) * data[i - 1];
  }

  std::cout << "value: " << value << "\n";

  auto cum_prob = to_cum_prob(prob);
  std::string ans = "";
  bool fl = true;
  double low = 0, high = 1;

  while (fl){
    double w = high - low;
    for (int i = 0; i < 127; ++i){
      if (low + w * cum_prob[i] <= value &&  value < low + w * cum_prob[i + 1]){
        ans += (char) i;
        high = low + w * cum_prob[i + 1];
        low = low + w * cum_prob[i];

        if (i == eof){
          fl = false;
        }
      }
    }
  }

  return ans;
}

int main(){
  std::unordered_map<char, double> test_prob;

  for (char c = 'a'; c <= 'z'; ++c){
    test_prob[c] = 0.9 / ('z' - 'a' + 1);
  }
  test_prob[eof] = 0.05;
  test_prob[' '] = 0.05;

  std::string test_msg;

  std::cin >> test_msg;

  auto ans = encode(test_msg, test_prob);

  std::cout << ans[0] << " " << ans[1] << "\n";
  std::flush(std::cout);
  
  auto bin = interval_to_binary(ans[0], ans[1]);

  for (int i = 0; i < bin.second; ++i){
    std::cout << bin.first[i];
  }
  std::cout << "\n";

  auto dec_s = decode(test_prob, bin.first, bin.second);

  std::cout << dec_s << "\n";

  return 0;
}
