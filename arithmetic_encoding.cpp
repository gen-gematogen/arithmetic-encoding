#include <climits>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

std::unordered_map<char, double> to_cum_prob(std::unordered_map<char, double> &prob){
  std::unordered_map<char, double> cum_prob;
  cum_prob[0] = 0;
  for (int c = 1; c <= 127; ++c){
    cum_prob[(char)c] = cum_prob[c - 1] + prob[c - 1];
  }
  return cum_prob;
}

std::vector<double> encode(std::string msg, std::unordered_map<char, double> prob){
  double low = 0, high = 1;
  auto cum_prob = to_cum_prob(prob);

//  std::cout << "---\n";
//  for (char c = 0; c < 127; ++c){
//    std::cout << c << "  " << prob[c] << " " << cum_prob[c] << "\n";
//  }
//  std::cout << "---\n";

  for (char l: msg){
    double new_low = high * cum_prob[l];
    double new_high = high * (l < 127 ? cum_prob[l + 1] : 1);
    low = new_low;
    high = new_high;
  }

  return {low, high};
}

int main(){
  std::unordered_map<char, double> test_prob;

  test_prob['a'] = 0.5;
  test_prob['b'] = 0.5;

  std::string test_msg;

  std::cin >> test_msg;

  auto ans = encode(test_msg, test_prob);

  std::cout << ans[0] << " " << ans[1] << "\n";

  return 0;
}
