
#pragma once

#include <string>
#include <optional>
#include <cmath>
#include "rational.h"
#include "pow.h"

using Error = std::string;

template<typename Number>
class Calculator
{
public:
    void Set(Number n){
        result_ = n;
    };

    Number GetNumber() const{
        return result_;
    };

    std::optional<Error> Add(Number n){
        result_ += n;
        return std::nullopt;
    };

    std::optional<Error> Sub(Number n){
        result_ -= n;
        return std::nullopt;
    };

    std::optional<Error> Div(Number n){
        if constexpr (std::is_integral_v<Number>){
            if(n == 0){
                return "Division by zero";
            }
            result_ /= n;
            return std::nullopt;
        }else if constexpr (std::is_same_v<Number, Rational>){
            if(n == 0){
                return "Division by zero";
            }
            result_ /= n;
            return std::nullopt;
        }else{
            result_ /= n;
            return std::nullopt;
        }
    };

    std::optional<Error> Mul(Number n){
        result_ *= n;
        return std::nullopt;
    };

    std::optional<Error> Pow(Number n){
        if constexpr (std::is_integral_v<Number>){
            if(result_ == 0 && n == 0){
                return "Zero power to zero";
            }
            if(n<0){
                return "Integer negative power";
            }
            result_ = IntegerPow(result_, n);
            return std::nullopt;
        }else if constexpr (std::is_same_v<Number, Rational>){
            if(result_ == 0 && n == 0){
                return "Zero power to zero";
            }
            if(n.GetDenominator() != 1){
                return "Fractional power is not supported";
            }
            result_ = ::Pow(result_, n);
            return std::nullopt;
        }else{
            result_ = pow(result_,n);
            return std::nullopt;
        }
    };

    void Save(){
        number_mem_ = result_;
    };

    void Load(){
        if(number_mem_.has_value()){
            result_ = number_mem_.value();
        }
    };

    bool GetHasMem() const{
        return number_mem_ != std::nullopt;
    };

    std::string GetNumberRepr() const{
        return std::to_string(result_);
    };

private:
    std::optional<Number> number_mem_ = std::nullopt;
    Number result_ = 0;
};
