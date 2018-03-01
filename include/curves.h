/*
 * curves.h
 * Friendly interface for computing on the BN254 curve via the RELIC library
 */
#pragma once

#include <string>
#include <vector>

extern "C" {
#include <relic/relic.h>
}

class G1;
class G2;
class GT;
GT pair_points(const G1& g1, const G2& g2);
std::vector<unsigned char> bn_to_bytes(bn_t x);

class Zp {
public:
    Zp();
    Zp(bn_t q);
    Zp(const Zp& m);
    Zp& operator=(const Zp& m);
    ~Zp();
    Zp(bn_t m, bn_t p);
    Zp(std::vector<unsigned char> s, bn_t q);

    void set_one();

    Zp& operator*=(const Zp& rhs);
    friend Zp operator*(Zp lhs, const Zp& rhs);

    Zp& operator+=(const Zp& rhs);
    friend Zp operator+(Zp lhs, const Zp& rhs);

    Zp& operator-=(const Zp& rhs);
    friend Zp operator-(Zp lhs, const Zp& rhs);
    Zp operator-();

    Zp operator%(Zp x);

    Zp randomize();

    std::string str();
    std::vector<unsigned char> bytes();

    friend class G1;
    friend class G2;
    friend class GT;

protected:
    void reset_n();
    void reset_p();
    void set_n(const bn_t m);
    void set_p(const bn_t q);
    bn_t n;
    bn_t p;
};

class G1 { 
public: 
    G1();
    G1(ep_t x);
    G1(const G1& q);
    G1& operator=(const G1& q);
    ~G1();
    G1(std::vector<unsigned char> s);
 
    G1& operator*=(const G1& rhs);
    friend G1 operator*(G1 lhs, const G1& rhs);
 
    G1 exp(Zp x) const;
    void randomize();

    std::string str();
    std::vector<unsigned char> bytes();
    bool is_id();

    friend GT pair_points(const G1& g1, const G2& g2);
 
protected:
    ep_t p; 
}; 

class G2 { 
public: 
    G2();
    G2(const G2& q); 
    G2(ep2_t x);
    G2& operator=(const G2& q);
    ~G2();
    G2(std::vector<unsigned char> s);
 
    G2& operator*=(const G2& rhs);
    friend G2 operator*(G2 lhs, const G2& rhs);
 
    G2 exp(Zp x) const;
    void randomize();

    std::string str();
    std::vector<unsigned char> bytes();
    bool is_id();

    friend GT pair_points(const G1& g1, const G2& g2);
 
protected:
    ep2_t p; 
}; 

class GT { 
public: 
    GT();
    GT(fp12_t x);
    GT(const GT& q) ;
    GT& operator=(const GT& q);
    ~GT();
    GT(std::vector<unsigned char> s);
 
    GT& operator*=(const GT& rhs);
    friend GT operator*(GT lhs, const GT& rhs);

    GT exp(Zp x) const;
    void randomize();
    
    std::string str();
    std::vector<unsigned char> bytes();
    bool is_id();
 
protected:
    fp12_t p; 
}; 

