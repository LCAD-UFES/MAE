#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

int
main(int argc, char** argv)
{
    string input_file;
    string output_file;
    ifstream input;
    ofstream output;

    if (argc != 3)
    {
        cerr << argv[0] << " output_info.txt input_info.txt" << endl;
        return -1;
    }

    input_file = argv[2];
    output_file = argv[1];

    cerr << "Opening input_file with names of images" << endl;
    input.open(input_file.c_str());
    cerr << "Done." << endl;

    output.open(output_file.c_str(), ofstream::out | ofstream::app);

    if (output.is_open() && input.is_open())
    {
        string line;
        getline(input, line);
        while (!input.eof())
        {
            string s;
            vector<string> strings;
            istringstream iss(line);
            while (getline(iss, s, ','))
            {
                cout << "Leitura: " << s << endl;
                strings.push_back(s);
            }
            output << strings.at(0) << ";" << strings.at(1) << ";" << strings.at(2);

            getline(input, line);
            strings.clear();

            istringstream isss(line);
            while (getline(isss, s, ','))
            {
                cout << "Leitura: " << s << endl;
                strings.push_back(s);
            }
            output << ";" << strings.at(1) << ";" << strings.at(2) << endl;

            getline(input, line);
        }
    }

    else
    {
        cerr << "Failed to open: " << input_file << endl;
    }

    input.close();
    output.close();

    return EXIT_SUCCESS;
}

