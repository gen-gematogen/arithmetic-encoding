#include <bitset>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <fstream>

typedef unsigned long long ull;

const static int N = 1000;
const static char eof = 0;
const static ull prec = 31;
const static ull whole = 1 << (prec - 1);
const static ull half = whole >> 1;
const static ull quart = whole >> 2;
const static ull R = 1<<30; 

std::unordered_map<char, ull> to_cum_prob(std::unordered_map<char, ull> &prob){
  std::unordered_map<char, ull> cum_prob;
  
  cum_prob[0] = 0;

  for (int c = 1; c < 128; ++c){
    cum_prob[(char)c] = cum_prob[c - 1] + prob[c - 1];
  }
  
  return cum_prob;
}

ull round_div(ull a, ull b){
  if (2 * (a % b) >= b){
    return a / b + 1;
  }
  return a / b;
}

std::pair<std::bitset<N>, int> encode(std::string msg, std::unordered_map<char, ull> prob){
  int k = 0;
  ull min = 0, max = whole, s = 0, w;
  std::bitset<N> bin_repr;
  
  bin_repr.reset();

  auto cum_prob = to_cum_prob(prob);

  for (char l: msg){
    w = max - min;
    max = min + w * cum_prob[l + 1] / R; // round_div(w * cum_prob[l + 1], R);
    min = min + w * cum_prob[l] / R; // round_div(w * cum_prob[l], R);
    
    while (max < half || min > half){
      if (max < half){
        min = min << 1;
        max = max << 1;
        
        bin_repr[k++] = 0;
        for (int i = 0; i < s; ++i){
          bin_repr[k++] = 1;
        }
        
        s = 0;
      }

      else if (min > half){
        min = (min - half) << 1;
        max = (max - half) << 1;
        
        bin_repr[k++] = 1;
        for (int i = 0; i < s; ++i){
          bin_repr[k++] = 0;
        }
        
        s = 0;
      }
    }

    while (min > quart && max < 3 * quart){
      min = (min - quart) << 1;
      max = (max - quart) << 1;
      s++;
    }
  }

  s++;

  if (min <= quart){
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

std::string decode(std::unordered_map<char, ull> prob, std::bitset<N> data, int k){
  int i = 0;
  ull min = 0, max = whole, value = 0, w;
  std::string ans = "";

  auto cum_prob = to_cum_prob(prob);

  for (; i < prec - 1 && i < N && i < k; ++i){
    if (data[i]){
      value += ((ull)1)<<(prec - i - 2);
    }
  }
  
  while (1) {
    for (int c = 0; c < 127; ++c){
      w = max - min;
      ull max_cur = min + w * cum_prob[c + 1] / R; // round_div(w * cum_prob[c + 1], R);
      ull min_cur = min + w * cum_prob[c] / R; //round_div(w * cum_prob[c], R);

      if (min_cur <= value && value < max_cur){
        ans += (char) c;
        min = min_cur;
        max = max_cur;

        if (c == eof){
          return ans;
        }

        break;
      }
    }

    while (max < half || min > half){
      if (max < half){
        min = min << 1;
        max = max << 1;
        value = value << 1;
      }
      else if (min > half){
        min = (min - half) << 1;
        max = (max - half) << 1;
        value = (value - half) << 1;
      }

      if (i < N && data[i]){
        value++;
      }
      i++;
    }

    while (min > quart && max < 3 * quart){
      min = (min - quart) << 1;
      max = (max - quart) << 1;
      value = (value - quart) << 1;

      if (i < N && data[i]){
        value++;
      }
      i++;
    }
  }

  return ans;
}

int main(int argc, char **argv){
  if (argc == 1){
    std::cerr << "Mode not specified\n";
    return 1;
  }

  if (argv[1] == "-e" || argv[1] == "--encode"){
    std::string input_file = argv[2];
    std::string output_file = argv[3];

    std::ifstream in_stream;
    in_stream.open(input_file, std::ios::in);

    std::stringstream data;
    data << in_stream.rdbuf();
    data << eof;

    in_stream.close();

    auto str_data = data.str();

    auto encoded_data = encode(std_data, prob);

  }
  else if (argv[1] == "-d" || argv[1] == "--decode"){

  }
  else{ 
    std::cerr << "No such mode. Please select -e/--encode or -d/--decode.\n";
    return 1;
  }

  std::unordered_map<char, ull> test_prob;

  // for (char c = 'a'; c <= 'z'; ++c){
  //   test_prob[c] = 0.9 / ('z' - 'a' + 1);
  // }
  test_prob['a'] = test_prob['b'] = (R - (R >> 7)) >> 1;
  test_prob[eof] = R >> 7;
  // test_prob[' '] = 0.05;

  std::string test_msg;

  std::cin >> test_msg;
  test_msg += eof;

  auto ans = encode(test_msg, test_prob);

  for (int i = 0; i < ans.second; ++i){
    std::cout << ans.first[i];
  }
  std::cout << "\n";
  std::fflush(stdout);

  auto dec_s = decode(test_prob, ans.first, ans.second);

  std::cout << dec_s << "\n";

  return 0;
}
