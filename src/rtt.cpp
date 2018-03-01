#include "rtt.h"
#include "mbm.h"

#include <string>
#include <vector>
#include <memory>

using namespace std;

tuple<RTT::MasterSecretKey, RTT::PublicKey, RTT::Params> RTT::setup(const long n, const long k) {
    MBM mbm(k+1);
    auto mbm_keypair = mbm.generate_keypair();
    RTT::MasterSecretKey msk(n, k, mbm_keypair.first);
    auto mpk = static_cast<RTT::PublicKey>(mbm_keypair.second);
    auto params = static_cast<RTT::Params>(mbm.params);

    return make_tuple(msk, mpk, params);
}

vector<RTT::SecretKey> RTT::gen_user_keys(const RTT::MasterSecretKey& msk, const RTT::PublicKey& mpk, const RTT::Params& params, const long n) {
    int vec_length = msk.mbm_msk.a.size();
    int k = vec_length - 1;
    MBM mbm(vec_length, static_cast<MBM::Params>(params));

    auto user_keys = vector<RTT::SecretKey>();
    vector<bool> x;
    for (int i = 0; i < n; i++) {
        x = gen_key_vector(k, msk.w, i);
        user_keys.push_back(mbm.keygen(msk.mbm_msk, x));
    }

    return user_keys;
}

RTT::Ciphertext RTT::encrypt(const RTT::PublicKey& mpk, const RTT::Params& params, const string ptxt) {
    int vec_length = mpk.k2.size();
    MBM mbm(vec_length, static_cast<MBM::Params>(params));

    return static_cast<RTT::Ciphertext>(mbm.encrypt(static_cast<MBM::PublicKey>(mpk), ptxt));
}

string RTT::decrypt(const RTT::SecretKey& sk, const RTT::Params& params, const RTT::Ciphertext& ctxt) {
    int vec_length = sk.k2.size();
    MBM mbm(vec_length, static_cast<MBM::Params>(params));

    return mbm.decrypt(static_cast<MBM::SecretKey>(sk), static_cast<MBM::Ciphertext>(ctxt));
}

vector<bool> RTT::gen_key_vector(const long k, const long w, const long i) {
    auto x = vector<bool>(k+1, false);

    if (i < w) {
        // do nothing
    } else if (i < w + k) {
        for (int j = k - i + w; j < k + 1; j++) {
            x.at(j - (k-i+w)) = true;
        }
    } else {
        x = vector<bool>(k+1, true);
    }

    return x;
}