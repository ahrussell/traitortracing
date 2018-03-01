#include <string>
#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "mbm.h"
#include "curves.h"
#include "util.h"

extern "C" {
#include <relic/relic.h>
#include <sodium.h>
}

using namespace std;
using json = nlohmann::json;

static void derive_key(uint8_t key[], GT& c);

/*
 * Generates a master public/private keypair
 */
pair<MBM::MasterSecretKey, MBM::PublicKey> MBM::generate_keypair() {
    Zp alpha = Zp(p).randomize();

    auto a = vector<Zp>();
    auto b = vector<Zp>();
    auto c = vector<Zp>();

    Zp ai(p);
    Zp bi(p);
    Zp ci(p);

    Zp k1_exp(p);

    auto k2 = vector<G1>();

    for (int i = 0; i < vec_length; i++) {
        ai = Zp(p).randomize();
        bi = Zp(p).randomize();
        ci = Zp(p).randomize();
        a.push_back(ai);
        b.push_back(bi);
        c.push_back(ci);

        k1_exp += bi * ai + ci;
        k2.push_back(g1->exp(ai));
    }

    G1 g = g1->exp(alpha);
    GT k0 = pair_points(g,*g2);

    G1 k1 = g1->exp(k1_exp);

    MBM::PublicKey mpk(k0, k1, k2);
    MBM::MasterSecretKey msk(alpha, a, b, c);

    return make_pair(msk, mpk);
}

/*
 * Generates a user's secret key
 */
MBM::SecretKey MBM::keygen(const MasterSecretKey& msk, const vector<bool> x) {
    if (x.size() != vec_length){
        throw invalid_argument("vector is not the right size");
    }

    Zp t = Zp(p).randomize();

    vector<G2> k2 = vector<G2>();

    Zp k0_exp = msk.alpha;
    Zp k2_exp(p);

    Zp ui(p);

    for(int i = 0; i < vec_length; i ++) {

        k0_exp += -t * msk.c.at(i);

        // -t * b_i
        k2_exp = -t * msk.b.at(i);

        if (!x.at(i)) {
            ui.randomize();
            k2_exp += ui;
            k0_exp += -ui * msk.a.at(i);
        }

        // k2_i = -t * b_i + u_i * x_i
        k2.push_back(g2->exp(k2_exp));
    }
    
    G2 k0 = g2->exp(k0_exp);
    G2 k1 = g2->exp(t);
    
    return MBM::SecretKey(k0, k1, k2, vec_length);
}

string MBM::decrypt(const MBM::SecretKey& sk, const MBM::Ciphertext& ctxt) {
    GT p0 = pair_points(ctxt.c0, sk.k0);
    GT p1 = pair_points(ctxt.c1, sk.k1);
    GT p2 = GT();
    for (int i = 0; i < vec_length; i++) {
        p2 *= pair_points(ctxt.c2.at(i), sk.k2.at(i));
    }

    GT p = p0 * p1 * p2;

    uint8_t key[crypto_secretbox_KEYBYTES];
    derive_key(key, p);

    const unsigned char* ctxt_arr = &ctxt.ctxt[0];
    const unsigned char* nonce_arr = &ctxt.nonce[0];

    size_t message_len = ctxt.ctxt.size() - crypto_secretbox_MACBYTES;

    auto decrypted = new unsigned char[message_len];
    if (crypto_secretbox_open_easy(decrypted, ctxt_arr, message_len + crypto_secretbox_MACBYTES, nonce_arr, key) != 0) {
        /* message forged! */
    }

    string message(reinterpret_cast<char*>(decrypted), message_len);

    delete[] decrypted;
    return message;
}

MBM::Ciphertext MBM::encrypt(const MBM::PublicKey& mpk, const string& plaintext) {
    Zp s(p);
    s.randomize();
    GT c = mpk.k0.exp(s);
    G1 c0 = g1->exp(s);
    G1 c1 = mpk.k1.exp(s);
    auto c2 = vector<G1>();

    for (int i = 0; i < vec_length; i++) {
        c2.push_back(mpk.k2.at(i).exp(s));
    }

    uint8_t key[crypto_secretbox_KEYBYTES];
    derive_key(key, c);

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);
    
    size_t ctxt_len = crypto_secretbox_MACBYTES + plaintext.length();
    unsigned char* ciphertext = new unsigned char[ctxt_len];
    const unsigned char* message = reinterpret_cast<const unsigned char*>(plaintext.c_str());
    size_t message_len = plaintext.length();

    // // encrypt plaintext
    crypto_secretbox_easy(ciphertext, message, message_len, nonce, static_cast<unsigned char*>(key));
    auto x = vector<unsigned char>(ciphertext, ciphertext + ctxt_len);
    auto y = vector<unsigned char>(nonce, nonce + crypto_secretbox_NONCEBYTES);
    delete[] ciphertext;
    return MBM::Ciphertext(x, y, c0, c1, c2);
}

/*
 * Derives a symmetric key for use with libsodium
 */
static void derive_key(uint8_t* key, GT& c) {
    string encaps_key = c.str();

    // derive key
    unsigned char hash[crypto_generichash_BYTES];
    const unsigned char* k = reinterpret_cast<const unsigned char*>(encaps_key.c_str());
    size_t k_len = encaps_key.length();
    crypto_generichash(hash, sizeof hash, k, k_len, nullptr, 0);
    uint8_t master_key[crypto_kdf_KEYBYTES];

    for (int i = 0; i < crypto_kdf_KEYBYTES; i++) {
        master_key[i] = static_cast<uint8_t>(hash[i]);
    }

    crypto_kdf_derive_from_key(key, crypto_secretbox_KEYBYTES, 1, "context", master_key);
}

/*
 * Serialization functions
 */

MBM::PublicKey::PublicKey(json pk_json) {
    k0 = GT(base64_decode(pk_json["k0"]));
    k1 = G1(base64_decode(pk_json["k1"]));

    k2 = vector<G1>();

    for (auto& c : pk_json["k2"]) {
        k2.push_back(G1(base64_decode(c)));
    }
}

json MBM::PublicKey::to_json() {
    json j, j2;
    j["k0"] = base64_encode(k0.bytes());
    j["k1"] = base64_encode(k1.bytes());

    for (auto& g : k2) {
        j2.push_back(base64_encode(g.bytes()));
    }

    j["k2"] = j2;

    return j;
}

MBM::SecretKey::SecretKey(json j) {
    k0 = G2(base64_decode(j["k0"]));
    k1 = G2(base64_decode(j["k1"]));

    k2 = vector<G2>();

    for (auto& c : j["k2"]) {
        k2.push_back(G2(base64_decode(c)));
    }
}

json MBM::SecretKey::to_json() {
    json j, j2;
    j["k0"] = base64_encode(k0.bytes());
    j["k1"] = base64_encode(k1.bytes());

    for (auto& g : k2) {
        j2.push_back(base64_encode(g.bytes()));
    }

    j["k2"] = j2;

    return j;
}

MBM::MasterSecretKey::MasterSecretKey(json j) {
    bn_t p;
    bn_null(p);
    bn_new(p);
    bn_zero(p);
    ep_curve_get_ord(p); 

    alpha = Zp(base64_decode(j["alpha"]), p);
    a = vector<Zp>();
    b = vector<Zp>();
    c = vector<Zp>();

    for (auto& ai : j["a"]) {
        a.push_back(Zp(base64_decode(ai), p));
    }

    for (auto& bi : j["b"]) {
        b.push_back(Zp(base64_decode(bi), p));
    }

    for (auto& ci : j["c"]) {
        c.push_back(Zp(base64_decode(ci), p));
    }
}

json MBM::MasterSecretKey::to_json() {
    json j, ja, jb, jc;
    j["alpha"] = base64_encode(alpha.bytes());

    for (auto& ai : a) {
        ja.push_back(base64_encode(ai.bytes()));
    }

    for (auto& bi : b) {
        jb.push_back(base64_encode(bi.bytes()));
    }

    for (auto& ci : c) {
        jc.push_back(base64_encode(ci.bytes()));
    }

    j["a"] = ja;
    j["b"] = jb;
    j["c"] = jc;

    return j;
}

MBM::Params::Params(json param_json) {
    auto g1_bytes = base64_decode(param_json["g1"]);
    g1 = make_shared<G1>(G1(g1_bytes));

    auto g2_bytes = base64_decode(param_json["g2"]);
    g2 = make_shared<G2>(G2(g2_bytes));

    vec_length = static_cast<size_t>(param_json["vec_length"]);

    set_curve_order();
}

json MBM::Params::to_json() {
    json j;
    j["g1"] = base64_encode(g1->bytes());
    j["g2"] = base64_encode(g2->bytes());
    j["vec_length"] = (int)vec_length;
    j["curve"] = curve;

    return j;
}

MBM::Ciphertext::Ciphertext(json ctxt_json) {
    auto c0_bytes = base64_decode(ctxt_json["c0"]);
    c0 = G1(c0_bytes);

    auto c1_bytes = base64_decode(ctxt_json["c1"]);
    c1 = G1(c1_bytes);

    c2 = vector<G1>();
    ctxt = base64_decode(ctxt_json["ctxt"]);
    nonce = base64_decode(ctxt_json["nonce"]);

    for (auto& c : ctxt_json["c2"]) {
        c2.push_back(G1(base64_decode(c)));
    }
}

json MBM::Ciphertext::to_json() {
    json j, j2;
    j["c0"] = base64_encode(c0.bytes());
    j["c1"] = base64_encode(c1.bytes());

    for (auto& g : c2) {
        j2.push_back(base64_encode(g.bytes()));
    }

    j["c2"] = j2;
    j["ctxt"] = base64_encode(ctxt);
    j["nonce"] = base64_encode(nonce);

    return j;
}
