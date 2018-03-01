#pragma once

#include "mbm.h"
#include <string>
#include <vector>
#include <memory>
#include <tuple>

#include <nlohmann/json.hpp>

extern "C" {
#include <sodium.h>
}

using namespace std;
using json = nlohmann::json;

class RTT {
public:
    typedef MBM::SecretKey SecretKey;
    typedef MBM::Ciphertext Ciphertext;
    typedef MBM::PublicKey PublicKey;
    typedef MBM::Params Params;

    class MasterSecretKey {
    public:
        MasterSecretKey(size_t n, size_t k, MBM::MasterSecretKey msk) :
                mbm_msk(msk) {
            uint32_t x = randombytes_uniform((n-1) + k - 1);
            w = x - k + 1;
        }

        MasterSecretKey(json j) : 
                mbm_msk(MBM::MasterSecretKey(j["mbm_msk"])), w(j["w"]) {

        }

        json to_json() {
            json j;
            j["mbm_msk"] = mbm_msk.to_json();
            j["w"] = w;

            return j;
        }

        MBM::MasterSecretKey mbm_msk;
        long w = 0;
    };

    static tuple<MasterSecretKey, PublicKey, Params> setup(const long n, const long k);
    static vector<SecretKey> gen_user_keys(const MasterSecretKey& msk, const PublicKey& mpk, const Params& params, const long n);
    static Ciphertext encrypt(const PublicKey& mpk, const Params& params, const string ptxt);
    static string decrypt(const SecretKey& sk, const Params& params, const Ciphertext& ctxt);
    
private:
    static vector<bool> gen_key_vector(const long n, const long w, const long i);
};