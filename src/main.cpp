#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

extern "C" {
#include <relic/relic.h>
#include <sodium.h>
}

#include "mbm.h"
#include "rtt.h"
#include "curves.h"
#include "cli.h"

using namespace std;
using json = nlohmann::json;

vector<string> modes = {"setup", "encrypt", "decrypt", "test"};

map<string, string> helps = {{"setup", "Setup takes as input a number of users and a riskiness level and writes parameters and keys to specified files."},
                             {"encrypt", "Encrypt takes as input 3 files containing parameters, a public key, and the plaintext, respectively, and writes the resulting ciphertext to a specified file."},
                             {"decrypt", "Decrypt takes as input 3 files containing parameters, a user's public key, and a ciphertext, respectively, and writes the resulting plaintext to a specified file."},
                             {"test", "Test mode generates parameters and keys, then encrypts and decrypts a test ciphertext."}};

map<string, map<string, string, cmp>> args = {{"setup",   {{"num_users", "the number of users"},
                                                           {"risky_level", "the riskiness level"},
                                                           {"params", "an output file for the scheme parameters"},
                                                           {"pk", "an output file for the public key"},
                                                           {"msk", "an output file for the master secret key"},
                                                           {"sks", "a directory to output the secret key files"}}},
                                              {"encrypt", {{"params", "an input file containing the scheme parameters"},
                                                           {"pk", "an input file containing the public key"},
                                                           {"ptxt", "an input file containing the plaintext"},
                                                           {"ctxt", "an output file for the ciphertext"}}},
                                              {"decrypt", {{"params", "an input file containing the scheme parameters"},
                                                           {"sk", "an input file containing a user's secret key"},
                                                           {"ctxt", "an input file containing the ciphertext"},
                                                           {"ptxt", "an output file for the plaintext"}}},
                                              {"test",    {}}
                                             };

int main(int argc, char* argv[]) {
    if (argc < 2) {
        display_help(argv[0], modes, helps, args);
        return 0;
    }

    string mode(argv[1]);
    
    bool valid_args = false;
    if (find(modes.begin(), modes.end(), mode) == modes.end()) {
        display_help(argv[0], modes, helps, args);
        valid_args = false;
    } else {
        valid_args = verify_args(argc, argv[0], mode, helps[mode], args[mode]);
    }

    if (!valid_args) {
        return 0;
    }

    // Initialize RELIC
    if (core_init() != STS_OK) {
		core_clean();
		return 1;
	}

    // init RELIC curve
	if (ep_param_set_any_pairf() == STS_ERR) {
		THROW(ERR_NO_CURVE);
		core_clean();
		return 0;
	}

    // Initialize libsodium
    if (sodium_init() == -1) {
        return 1;
    }

    if (mode == "setup") {
        int n = stoi(argv[2]);
        int k = stoi(argv[3]);

        auto output = RTT::setup(n, k);
        auto msk = get<0>(output);
        auto mpk = get<1>(output);
        auto params = get<2>(output);

        ofstream paramfile;
        paramfile.open(argv[4]);
        paramfile << params.to_json();
        paramfile.close();

        ofstream pkfile(argv[5]);
        pkfile << mpk.to_json();
        pkfile.close();

        ofstream mskfile(argv[6]);
        mskfile << msk.to_json();
        mskfile.close();

        auto user_sk = RTT::gen_user_keys(msk, mpk, params, n);
        string sk_dir(argv[7]);
        if (sk_dir.back() != '/') {
            sk_dir += "/";
        }
        
        int i = 0;
        for (auto& sk : user_sk) {
            ofstream skfile(sk_dir + "sk_" + to_string(i) + ".json");
            skfile << sk.to_json();
            skfile.close();
            i++;
        }
    } else if (mode == "encrypt") {
        ifstream paramfile(argv[2]);
        ifstream pkfile(argv[3]);
        ifstream ptxtfile(argv[4]);

        stringstream paramss;
        stringstream pkss;
        stringstream ptxtss;
        paramss << paramfile.rdbuf();
        pkss << pkfile.rdbuf();
        ptxtss << ptxtfile.rdbuf();

        paramfile.close();
        ptxtfile.close();
        pkfile.close();

        RTT::Params params(json::parse(paramss.str()));
        RTT::PublicKey pk(json::parse(pkss.str()));
        auto ctxt = RTT::encrypt(pk, params, ptxtss.str());

        ofstream ctxtfile(argv[5]);
        ctxtfile << ctxt.to_json();
        ctxtfile.close();
    } else if (mode == "decrypt") {
        ifstream paramfile(argv[2]);
        ifstream skfile(argv[3]);
        ifstream ctxtfile(argv[4]);

        stringstream paramss;
        stringstream skss;
        stringstream ctxtss;
        paramss << paramfile.rdbuf();
        skss << skfile.rdbuf();
        ctxtss << ctxtfile.rdbuf();

        paramfile.close();
        ctxtfile.close();
        skfile.close();

        RTT::Params params(json::parse(paramss.str()));
        RTT::SecretKey sk(json::parse(skss.str()));
        RTT::Ciphertext ctxt(json::parse(ctxtss.str()));

        auto ptxt = RTT::decrypt(sk, params, ctxt);

        ofstream ptxtfile(argv[5]);
        ptxtfile << ptxt;
        ptxtfile.close();
    } else if (mode == "test") {
        int n = 3;
        int k = 1;

        cout << "Generating parameters and keys..." << endl;

        auto output = RTT::setup(n, k);
        auto msk = get<0>(output);
        auto mpk = get<1>(output);
        auto params = get<2>(output);

        auto user_sk = RTT::gen_user_keys(msk, mpk, params, n);

        string plaintext("'Twas brillig, and the slithy toves\nDid gyre and gimble in the wabe:\nAll mimsy were the borogoves,\nAnd the mome raths outgrabe.");

        cout << "Test plaintext is: " << endl;
        cout << plaintext << endl << endl;

        cout << "Encrypting..." << endl;

        auto ctxt = RTT::encrypt(mpk, params, plaintext);

        cout << "Decrypting..." << endl;

        auto ptxt = RTT::decrypt(user_sk.at(0), params, ctxt);

        cout << "Decrypted plaintext is: " << endl;
    
        cout << ptxt << endl;
    }

    return 0;
}
