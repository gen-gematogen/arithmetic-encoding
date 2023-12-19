#include <bitset>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <vector>

typedef unsigned long long ull;

const static char eof = 'c';
const static ull prec = 31;
const static ull whole = 1 << (prec - 1);
const static ull half = whole >> 1;
const static ull quart = whole >> 2;
const static ull R = 1 << 30;

class data_buf
{
public:
  int w_bit = 0, r_bit = 0, r_len = 0;
  std::vector<std::bitset<8>> buf;

  data_buf()
  {
  }

  void add(bool v)
  {
    if (!buf.size())
    {
      buf.push_back(std::bitset<8>());
    }

    buf[buf.size() - 1][w_bit] = v;

    w_bit++;

    if (w_bit == 8)
    {
      buf.push_back(std::bitset<8>());
      w_bit = 0;
    }
  }

  std::vector<std::bitset<8>> get()
  {
    return buf;
  }

  int size()
  {
    return buf.size();
  }

  void append(std::bitset<8> b)
  {
    buf.push_back(b);
  }

  bool next()
  {
    bool v = buf[r_len][r_bit];

    r_bit++;

    if (r_bit == 8)
    {
      r_bit = 0;
      r_len++;
    }

    return v;
  }

  void reset_read()
  {
    r_bit = 0;
    r_len = 0;
  }

  ~data_buf()
  {
  }
};

std::unordered_map<char, ull> to_cum_prob(std::unordered_map<char, ull> &prob)
{
  std::unordered_map<char, ull> cum_prob;

  cum_prob[0] = 0;

  for (int c = 1; c < 128; ++c)
  {
    cum_prob[(char)c] = cum_prob[c - 1] + prob[c - 1];
  }

  return cum_prob;
}

ull round_div(ull a, ull b)
{
  if (2 * (a % b) >= b)
  {
    return a / b + 1;
  }
  return a / b;
}

data_buf encode(std::string msg, std::unordered_map<char, ull> prob)
{
  int k = 0;
  ull min = 0, max = whole, s = 0, w;
  data_buf buf;

  auto cum_prob = to_cum_prob(prob);

  for (char l : msg)
  {
    w = max - min;
    max = min + w * cum_prob[l + 1] / R; // round_div(w * cum_prob[l + 1], R);
    min = min + w * cum_prob[l] / R;     // round_div(w * cum_prob[l], R);

    while (max < half || min > half)
    {
      if (max < half)
      {
        min = min << 1;
        max = max << 1;

        buf.add(0);
        for (int i = 0; i < s; ++i)
        {
          buf.add(1);
        }

        s = 0;
      }

      else if (min > half)
      {
        min = (min - half) << 1;
        max = (max - half) << 1;

        buf.add(1);
        for (int i = 0; i < s; ++i)
        {
          buf.add(0);
        }

        s = 0;
      }
    }

    while (min > quart && max < 3 * quart)
    {
      min = (min - quart) << 1;
      max = (max - quart) << 1;
      s++;
    }
  }

  s++;

  if (min <= quart)
  {
    buf.add(0);
    for (int i = 0; i < s; ++i)
    {
      buf.add(1);
    }
  }
  else
  {
    buf.add(1);
    for (int i = 0; i < s; ++i)
    {
      buf.add(0);
    }
  }

  return buf;
}

std::string decode(data_buf data, std::unordered_map<char, ull> prob)
{
  int i = 0, N = data.size() << 3;
  ull min = 0, max = whole, value = 0, w;
  std::string ans = "";

  auto cum_prob = to_cum_prob(prob);

  for (; i < prec - 1 && i < N; ++i)
  {
    if (data.next())
    {
      value += ((ull)1) << (prec - i - 2);
    }
  }

  while (1)
  {
    for (int c = 0; c < 127; ++c)
    {
      w = max - min;
      ull max_cur = min + w * cum_prob[c + 1] / R; // round_div(w * cum_prob[c + 1], R);
      ull min_cur = min + w * cum_prob[c] / R;     // round_div(w * cum_prob[c], R);

      if (min_cur <= value && value < max_cur)
      {
        ans += (char)c;
        min = min_cur;
        max = max_cur;

        if (c == eof)
        {
          std::cout << N << " " << i << "\n";
          return ans;
        }

        break;
      }
    }

    while (max < half || min > half)
    {
      if (max < half)
      {
        min = min << 1;
        max = max << 1;
        value = value << 1;
      }
      else if (min > half)
      {
        min = (min - half) << 1;
        max = (max - half) << 1;
        value = (value - half) << 1;
      }

      if (i < N && data.next())
      {
        value++;
      }
      i++;
    }

    while (min > quart && max < 3 * quart)
    {
      min = (min - quart) << 1;
      max = (max - quart) << 1;
      value = (value - quart) << 1;

      if (i < N && data.next())
      {
        value++;
      }
      i++;
    }
  }

  return ans;
}

int main(int argc, char **argv)
{
  std::unordered_map<char, ull> prob;

  prob['a'] = prob['b'] = (R - (R >> 7)) >> 1;
  prob[eof] = R >> 7;

  if (argc == 1)
  {
    std::cerr << "Mode not specified\n";
    return 1;
  }

  if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--encode") == 0)
  {
    std::string input_file = argv[2];
    std::string output_file = argv[3];

    std::cout << "Opening file...\n";

    std::ifstream in_stream;
    in_stream.open(input_file, std::ios::in);

    std::cout << "Reading data...\n";

    std::stringstream data;
    data << in_stream.rdbuf();

    in_stream.close();

    auto str_data = data.str();
    str_data[str_data.size() - 1] = eof;

    std::cout << "Encoding...\n";

    auto encoded_buf = encode(str_data, prob);
    auto encoded_str = encoded_buf.get();

    std::cout << "Writing data to binary file...\n";

    std::ofstream out_stream;
    out_stream.open(output_file, std::ios::out | std::ios::binary);

    for (int i = 0; i < encoded_str.size(); ++i)
    {
      out_stream << (char)encoded_str[i].to_ullong();
    }

    out_stream.close();

    std::cout << "Done!\n";

    return 0;
  }
  else if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--decode") == 0)
  {
    std::string input_file = argv[2];
    std::string output_file = argv[3];

    std::cout << "Opening file...\n";

    std::ifstream in_stream;
    in_stream.open(input_file, std::ios::in | std::ios::binary);

    std::cout << "Reading data...\n";

    unsigned char c;
    data_buf encoded_data;

    in_stream >> c;
    while (!in_stream.eof())
    {
      std::bitset<8> cur((ull)c);
      encoded_data.append(cur);
      in_stream >> c;
    }

    in_stream.close();

    std::cout << "Decoding...\n";

    auto decoded_str = decode(encoded_data, prob);

    std::cout << "Writing data to output file...\n";

    std::ofstream out_stream;
    out_stream.open(output_file, std::ios::out);
    decoded_str.pop_back();

    out_stream << decoded_str;

    out_stream.close();

    std::cout << "Done!\n";

    return 0;
  }
  else
  {
    std::cerr << "No such mode. Please select -e/--encode or -d/--decode.\n";
    return 1;
  }

  return 0;
}
