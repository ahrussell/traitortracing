#include "curves.h"
#include "relic_compat.h"
#include "util.h"
#include <assert.h>
#include <string>
#include <vector>

#include <iostream>

extern "C" {
#include <relic/relic.h>
}

#define COMPACT 1

using namespace std;

vector<unsigned char> bn_to_bytes(bn_t x) {
    int len = bn_size_bin(x);
    uint8_t* buf = new uint8_t[len];
    bn_write_bin(buf, len, x);

    unsigned char* b = static_cast<unsigned char*>(buf);

    return vector<unsigned char>(b, b + len);
}

Zp::Zp() {
    reset_p();
    ep_curve_get_ord(p);
    reset_n();
}

Zp::Zp(bn_t q) { 
    reset_p();
    set_p(q);
    reset_n();
} 

Zp::Zp(bn_t m, bn_t q) { 
    reset_p();
    reset_n();
    set_p(q);
    set_n(m);
} 

Zp::Zp(const Zp& m) { 
    reset_p();
    reset_n();
    set_p(m.p);
    set_n(m.n);
} 

Zp& Zp::operator=(const Zp& m) { 
    set_p(m.p);
    set_n(m.n);
    return *this;
} 

Zp::~Zp() { 
    bn_free(n); 
    bn_free(p); 
}

Zp::Zp(vector<unsigned char> s, bn_t q) {
    bn_t x;
    bn_null(x); 
    bn_new(x);
    bn_read_bin(x, &s[0], s.size());

    set_p(q);
    set_n(x);
    bn_free(x);
}

void Zp::set_one() {
    // sets n to be 1
    bn_set_2b(n,0);
}

Zp& Zp::operator*=(const Zp& rhs) {                          
    assert(bn_cmp(p,rhs.p) == CMP_EQ);
    bn_mul(n,n,rhs.n);
    bn_mod(n,n,p);
    return *this; 
}

Zp operator*(Zp lhs, const Zp& rhs) {
    lhs *= rhs; 
    return lhs; 
}

Zp& Zp::operator+=(const Zp& rhs) {
    assert(bn_cmp(p,rhs.p) == CMP_EQ);
    bn_add(n,n,rhs.n);
    bn_mod(n,n,p);
    return *this; 
}

Zp operator+(Zp lhs, const Zp& rhs) {
    lhs += rhs; 
    return lhs; 
}

Zp& Zp::operator-=(const Zp& rhs) {                          
    assert(bn_cmp(p,rhs.p) == CMP_EQ);
    bn_t temp;
    bn_null(temp);
    bn_new(temp);
    bn_neg(temp,rhs.n);
    bn_add(n,n,temp);
    bn_mod(n,n,p);
    bn_free(temp);
    return *this; 
}

Zp operator-(Zp lhs, const Zp& rhs) {
    lhs -= rhs; 
    return lhs; 
}

Zp Zp::operator-() {
    Zp z(p);
    Zp x(p);
    x = z - *this;
    return x;
}

Zp Zp::operator%(Zp x) { 
    bn_mod(n,n,x.n); 
    bn_mod(n,n,p);
    return *this;
}

Zp Zp::randomize() {
    bn_rand(n, BN_POS, bn_bits(p)); 
    bn_mod(n,n,p);
    return *this;
}

string Zp::str() {
    return bn_to_str(n);
}

vector<unsigned char> Zp::bytes() {
    return bn_to_bytes(n);
}

void Zp::reset_n() {
    bn_null(n); 
    bn_new(n);
    bn_zero(n);
}

void Zp::reset_p() {
    bn_null(p);
    bn_new(p);
}

void Zp::set_n(const bn_t m) {
    bn_copy(n,m);
    bn_mod(n,n,p);
}

void Zp::set_p(const bn_t q) {
    bn_copy(p,q);
}



G1::G1() { 
    ep_null(p); 
    ep_new(p); 
    ep_set_infty(p);
} 

G1::G1(ep_t x) { 
    ep_null(p); 
    ep_new(p); 
    ep_copy(p, x); 
} 

G1::G1(const G1& q) { 
    ep_null(p); 
    ep_new(p); 
    ep_copy(p,q.p); 
} 

G1::G1(std::vector<unsigned char> s) {
    ep_null(p);
    ep_new(p);
    
    ep_read_bin(p, &s[0], s.size());
}

G1& G1::operator=(const G1& q) { 
    ep_copy(p,q.p); 
    return *this;
} 

G1::~G1() { 
    ep_free(p); 
} 

G1& G1::operator*=(const G1& rhs) {                          
    ep_add(p,p,rhs.p);
    return *this; 
}

G1 operator*(G1 lhs, const G1& rhs) {
    lhs *= rhs; 
    return lhs; 
}

G1 G1::exp(Zp x) const { 
    ep_t temp;
    ep_null(temp);
    ep_new(temp);
    ep_mul(temp,p,x.n);
    G1 g(temp);
    ep_free(temp);
    return g;
} 

void G1::randomize() {
    ep_rand(p);
}

string G1::str() {
    string s = "";

    s += fp_to_str(p->x);
    s += "\n";
    s += fp_to_str(p->y);
    s += "\n";
    s += fp_to_str(p->z);
    s += "\n";

    return s;
}

vector<unsigned char> G1::bytes() {
    int len = ep_is_infty(p) ? 1 : FP_BYTES + 1;
    uint8_t* buf = new uint8_t[len];
    ep_write_bin(buf, len, p, COMPACT);

    unsigned char* b = static_cast<unsigned char*>(buf);
    return vector<unsigned char>(b, b + len);
}



G2::G2() { 
    ep2_null(p); 
    ep2_new(p); 
    ep2_set_infty(p); 
} 

G2::G2(const G2& q) { 
    ep2_null(p); 
    ep2_new(p); 
    ep2_copy_const(p,q.p); 
} 

G2::G2(ep2_t x) { 
    ep2_null(p); 
    ep2_new(p); 
    ep2_copy(p, x); 
} 

G2& G2::operator=(const G2& q) {
    ep2_copy_const(p,q.p);
    return *this;
} 

G2::G2(std::vector<unsigned char> s) {
    ep2_null(p);
    ep2_new(p);
    
    ep2_read_bin(p, &s[0], s.size());
}

G2::~G2() { 
    ep2_free(p); 
} 

G2& G2::operator*=(const G2& rhs) {                          
    ep2_t temp;
    ep2_null(temp);
    ep2_new(temp);
    ep2_copy_const(temp,rhs.p);
    ep2_add(p,p,temp);
    ep2_free(temp); 
    return *this;
}

G2 operator*(G2 lhs, const G2& rhs) {
    lhs *= rhs; 
    return lhs; 
}

G2 G2::exp(Zp x) const {
    ep2_t temp;
    ep2_null(temp);
    ep2_new(temp);
    ep2_copy_const(temp, p);
    ep2_mul(temp,temp,x.n);
    G2 g(temp);
    ep2_free(temp);
    return g;
} 

void G2::randomize() {
    ep2_rand(p);
}

string G2::str() {
    string s = "";

    s += fp2_to_str(p->x);
    s += fp2_to_str(p->y);
    s += fp2_to_str(p->z);

    return s;
}

vector<unsigned char> G2::bytes() {
    int len = ep2_is_infty(p) ? 1 : 2 * FP_BYTES + 1;
    uint8_t* buf = new uint8_t[len];
    ep2_write_bin(buf, len, p, COMPACT);

    unsigned char* b = static_cast<unsigned char*>(buf);
    return vector<unsigned char>(b, b + len);
}


GT::GT() { 
    fp12_null(p); 
    fp12_new(p); 
    gt_set_unity(p);
} 

GT::GT(fp12_t x) { 
    fp12_null(p); 
    fp12_new(p); 
    fp12_copy(p, x); 
} 

GT::GT(const GT& q) { 
    fp12_null(p); 
    fp12_new(p); 
    fp12_copy_const(p,q.p); 
} 

GT::GT(std::vector<unsigned char> s) {
    fp12_null(p);
    fp12_new(p);
    
    fp12_read_bin(p, &s[0], s.size());
}

GT& GT::operator=(const GT& q) { 
    fp12_copy_const(p,q.p); 
    return *this;
} 

GT::~GT() { 
    fp12_free(p); 
} 

GT& GT::operator*=(const GT& rhs) {                          
    fp12_t temp;
    fp12_null(temp);
    fp12_new(temp);
    fp12_copy_const(temp, rhs.p);
    fp12_mul(p,p,temp);
    fp12_free(temp);
    return *this; 
}

GT operator*(GT lhs, const GT& rhs) {
    lhs *= rhs; 
    return lhs; 
}

GT GT::exp(Zp x) const { 
    fp12_t temp;
    fp12_null(temp);
    fp12_new(temp);
    fp12_copy_const(temp, p);
    fp12_exp(temp,temp,x.n);
    GT g(temp);
    fp12_free(temp);
    return g;
} 

void GT::randomize() {
    fp12_rand(p);
}

string GT::str() {
    return fp12_to_str(p);
}

vector<unsigned char> GT::bytes() {
    int len = 8 * FP_BYTES;
    uint8_t* buf = new uint8_t[len];
    fp12_write_bin(buf, len, p, COMPACT);

    unsigned char* b = static_cast<unsigned char*>(buf);
    return vector<unsigned char>(b, b + len);
}

GT pair_points(const G1& g1, const G2& g2) {
    fp12_t gt;
    fp12_null(gt);
    fp12_new(gt);

    ep_t temp;
    ep_null(temp);
    ep_new(temp);
    ep_copy(temp,g1.p);

    ep2_t temp2;
    ep2_null(temp2);
    ep2_new(temp2);
    ep2_copy_const(temp2,g2.p);

    pp_map_k12(gt, temp, temp2);

    GT P(gt);

    fp12_free(gt);
    return P;
}

