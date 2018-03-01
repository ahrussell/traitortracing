/*
 * mbm.h
 * Defines a Mixed Bit-Matching encryption scheme using the BN254 curve
 */
#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

extern "C" {
#include <relic/relic.h>
}

#include "curves.h"

using namespace std;
// for convenience
using json = nlohmann::json;


class MBM {

public:
    class PublicKey {
    public:
        GT k0;
        G1 k1;
        vector<G1> k2;

        PublicKey(GT pk0, G1 pk1, vector<G1> pk2) : k0(pk0), k1(pk1), k2(pk2) {
        }

        PublicKey(json pk_json);

        json to_json();
    };

    class SecretKey {
    public:
        SecretKey(G2 sk0, G2 sk1, vector<G2> sk2, size_t l) : vec_length(l), k0(sk0), k1(sk1), k2(sk2) {

        }

        SecretKey(json j);

        json to_json();

        G2 k0;
        G2 k1;
        vector<G2> k2;
        size_t vec_length;
    };

    class MasterSecretKey {
    public:
        MasterSecretKey (Zp al, vector<Zp> as, vector<Zp> bs, vector<Zp> cs) :
            alpha(al), a(as), b(bs), c(cs) {

        }

        MasterSecretKey(json j);

        json to_json();

        Zp alpha;
        vector<Zp> a;
        vector<Zp> b;
        vector<Zp> c;
    };

    class Params {
    public:
        Params(shared_ptr<G1> h1, shared_ptr<G2> h2, size_t l, bn_t q) : 
                                   g1(h1), g2(h2), vec_length(l) {
            bn_null(p);
            bn_new(p);
            bn_copy(p,q);
        }

        Params(size_t l) : vec_length(l) {
            g1 = make_shared<G1>();
            g1->randomize();
            g2 = make_shared<G2>();
            g2->randomize();

            set_curve_order();
        }

        Params(json param_json);

        json to_json();

        shared_ptr<G1> g1;
        shared_ptr<G2> g2;
        size_t vec_length;
        bn_t p;
        string curve = "BN254";

    private:
        void set_curve_order() {
            bn_null(p);
            bn_new(p);
            bn_zero(p);
            ep_curve_get_ord(p); 
        }
    };

    class Ciphertext {
    public:
        Ciphertext(vector<unsigned char> ciphertext, vector<unsigned char> n, G1 x0, G1 x1, vector<G1> x2) :
                        ctxt(ciphertext), nonce(n), c0(x0), c1(x1), c2(x2) {
        }

        Ciphertext(json ctxt_json);

        json to_json();

        G1 c0;
        G1 c1;
        vector<G1> c2;
        vector<unsigned char> ctxt;
        vector<unsigned char> nonce;
    };

    MBM(size_t l) : MBM(l, Params(l)) {

    } 

    MBM(size_t l, Params param) : params(param), vec_length(l) {
        g1 = params.g1;
        g2 = params.g2;
        bn_null(p);
        bn_new(p)
        bn_copy(p,params.p);
    } 

    pair<MasterSecretKey, PublicKey> generate_keypair();
    SecretKey keygen(const MasterSecretKey& msk, const vector<bool> x);
    string decrypt(const SecretKey& sk, const Ciphertext& ctxt);
    Ciphertext encrypt(const PublicKey& mpk, const string& plaintext);

    Params params;
    shared_ptr<G1> g1;
    shared_ptr<G2> g2;
    bn_t p;
    size_t vec_length;
};
