﻿#pragma once

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <sstream>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "tlv_types.h"

namespace tlv
{
//////////////////////////////////////////////////////////////////////////////////

class BytesBuffer
{
    public:
    BytesBuffer(uint8_t* data, size_t size): _data(data), _size(size) {}
    inline uint8_t* peek(size_t n)
    {
        if(n> _size || _size == 0)
        {
            return nullptr;
        }
        _data = _data + n;
        _size = _size - n;
        return _data;
    }
    inline size_t size() const {return _size;}
    inline uint8_t* data() const {return _data;}
    private:
    uint8_t* _data{nullptr};
    size_t _size {0};
};

/**
 * @brief 单元值
 *
 */
class TlvValue
{
public:
    /**
     * @brief 构造空对象
     *
     */
    TlvValue();
    
    /**
     * @brief 使用整型构造对象
     *
     * @param v
     */
    TlvValue(int v);
    
    /**
     * @brief 使用长整型构造对象
     *
     * @param v
     */
    TlvValue(int64_t v);
    TlvValue(uint64_t v);

    /**
     * @brief 使用无符号整型构造对象
     *
     * @param v
     */
    TlvValue(uint32_t v);
    
    /**
     * @brief 使用浮点数构造对象
     *
     * @param v
     */
    TlvValue(double v);
    
    /**
     * @brief 使用字符串构造对象
     *
     * @param v
     */
    TlvValue(const std::string &v);
    
    TlvValue(const TlvValue &cv);
    
    TlvValue(TlvValue &&cv);
    
    TlvValue &operator=(const TlvValue &cv);
    
    TlvValue &operator=(TlvValue &&cv) noexcept;
    
    ~TlvValue();
    
    /**
     * @brief 清空对象
     *
     */
    void clear();
    
    /**
     * @brief 设置对象值
     *
     * @param v
     */
    void set_value(int v);
    
    void set_value(int64_t v);
    void set_value(uint64_t v);

    void set_value(uint32_t v);
    
    void set_value(double v);
    
    void set_value(const std::string &v);
    void set_value(const char* v, size_t size);

    int to_int() const;

    void set_ptr(void* ptr, uint8_t dtype);
    
    /**
     * @brief 获取整型值, 自动类型转换
     *
     * @return int64_t
     */
    int64_t to_long() const;
    int64_t to_ulong() const;

    /**
     * @brief 获取浮点值, 自动类型转换
     *
     * @return double
     */
    double to_double() const;
    
    /**
     * @brief 获取字符串值, 自动类型转换
     *
     * @return std::string
     */
    std::string to_string() const;
    
    /**
     * @brief 获取内部字符串值，不进行类型转换
     *
     * @return const std::string&
     */
    // const std::string& get_string() const;
    
    const char *c_str() const;
    
    const char *data() const;
    
    size_t size() const;
    
        template<typename T> 
    const T* as_object() const
    {
        if(_dtype > 4)
        {
            return reinterpret_cast<const T*>(_sval);
        }
        else 
        {
            throw std::invalid_argument("invalid object pointer");
        }
    }

    template<typename T> 
    const T as_object_ref() const
    {
        if(_dtype > 4)
        {
            return *(reinterpret_cast<const T*>(_sval));
        }
        else 
        {
            throw std::invalid_argument("invalid object pointer");
        }
    }

    /**
     * @brief 获取值数据类型
     *    0: 空， 1：浮点型， 2：整型， 3: 字符串
     * @return int
     */
    int dtype() const;
    
    /**
     * @brief 是否为空
     *
     * @return true
     * @return false
     */
    bool is_none() const;
    
    /**
     * @brief 是否浮点值
     *
     * @return true
     * @return false
     */
    bool is_double() const;
    
    /**
     * @brief 是否整型值
     *
     * @return true
     * @return false
     */
    bool is_long() const;
    
    /**
     * @brief 是否字符串
     *
     * @return true
     * @return false
     */
    bool is_string() const;
    
    friend std::ostream &operator<<(std::ostream &oss, const TlvValue &o);

    uint8_t wrie_type() const;

    int serialize(uint16_t tag,std::string& out) const;

    /**
     * @brief 
     * 
     * @param in 
     * @param tag 
     * @return int != 0 fail； ==0 success 
     */
    // int deserialize(const std::string& in);
    int deserialize(BytesBuffer* in, uint16_t &tag);

    // array<object>
    //static int serialize_array0(uint16_t tag, const std::vector<TlvObject>& values,std::string& out);

    static int serialize_bytes(uint16_t tag, const char* values, size_t size,std::string& out);

    static int serialize_array2(uint16_t tag, const std::vector<int16_t>& values,std::string& out);
    static int serialize_array2(uint16_t tag, const std::vector<uint16_t>& values,std::string& out);
    static int serialize_array4(uint16_t tag, const std::vector<int32_t>& values,std::string& out);
    static int serialize_array4(uint16_t tag, const std::vector<uint32_t>& values,std::string& out);
    static int serialize_array4(uint16_t tag, const std::vector<float>& values,std::string& out);
    static int serialize_array8(uint16_t tag, const std::vector<int64_t>& values,std::string& out);
    //static int serialize_array8(uint16_t tag, const std::vector<uint64_t>& values,std::string& out);
    static int serialize_array8(uint16_t tag, const std::vector<double>& values,std::string& out);
    static int serialize_object(uint16_t tag, const std::vector<double>& values,std::string& out);


private:
    union
    {
        char _buffer[sizeof(int64_t)];
        double _fval;
        int64_t _ival;
        uint64_t _uval;
        char *_sval;
    };
    char _padding[sizeof(int32_t)]; // padding
    uint32_t _size: 24;            // string size
    uint32_t _dtype: 7;            // 0 null, 1 double, 2 int64, 3 string, 100 tlv_object, 101 tlv_array
    uint32_t _iscopy: 1;           // the string is copy, need free
};

inline void TlvValue::clear()
{
    if (_iscopy)
    {
        free(_sval);
    }
    _size = 0;
    _dtype = 0;
    _iscopy = 0;
    _sval = nullptr;
}

// inline const std::string& TlvValue::get_string() const {return _value;}
inline int TlvValue::dtype() const
{
    return _dtype;
}
inline uint8_t TlvValue::wrie_type() const
{
    switch (_dtype) {
    case 0:
        return TLV_LTYPE_NULL; //null
    case 1:
        if(_fval == 0.0)
        {
            return TLV_LTYPE_FALSE; // false
        }
        else if(_fval == 1.0)
        {
            return TLV_LTYPE_TRUE; // true
        }
        else if(std::isnan(_fval))
        {
            return TLV_LTYPE_NAN;
        }
        else if(std::isinf(_fval))
        {
            return TLV_LTYPE_INF_POS;
        }
        return TLV_LTYPE_FLOAT64;
    case 2:
        if(_ival == 0)
        {
            return TLV_LTYPE_FALSE;//
        }
        else if(_ival == 1)
        {
            return TLV_LTYPE_TRUE;//
        }
        else if(_ival > 0)
        {
           return TLV_LTYPE_VARINT_POS;// +int
        }
        return TLV_LTYPE_VARINT_NEG; // -int
    case 3:
        return TLV_LTYPE_BYTES;
    default:
        return TLV_LTYPE_ERROR; // error , unsupported data type
    }
}

inline const char *TlvValue::c_str() const
{
    if (_iscopy)
    {
        return _sval;
    }
    else
    {
        return _buffer;
    }
}

inline const char *TlvValue::data() const
{
    return c_str();
}

inline size_t TlvValue::size() const
{
    if (_dtype == 3)
    {
        return _size;
    }
    else if (_dtype == 0)
    {
        return 0;
    }
    return 8;
}

inline void TlvValue::set_value(double v)
{
    clear();
    _dtype = 1;
    _fval = v;
}

inline void TlvValue::set_value(int v)
{
    clear();
    _dtype = 2;
    _ival = v;
}

inline void TlvValue::set_value(int64_t v)
{
    clear();
    _dtype = 2;
    _ival = v;
}

inline void TlvValue::set_value(uint64_t v)
{
    clear();
    _dtype = 2;
    _uval = v;
}

inline void TlvValue::set_value(uint32_t v)
{
    clear();
    _dtype = 2;
    _ival = v;
}

inline bool TlvValue::is_none() const
{
    return _dtype == 0;
}

inline bool TlvValue::is_double() const
{
    return _dtype == 1;
}

inline bool TlvValue::is_long() const
{
    return _dtype == 2;
}

inline bool TlvValue::is_string() const
{
    return _dtype == 3;
}

inline bool operator<(const TlvValue &left, const TlvValue &right)
{
    if (left.dtype() == right.dtype())
    {
        if (left.is_double())
        {
            return left.to_double() < right.to_double();
        }
        else if (left.is_long())
        {
            return left.to_long() < right.to_long();
        }
        else
        {
            return ::strcmp(left.c_str(), right.c_str()) < 0;
        }
    }
    throw std::logic_error("dtype mismatch");
}

inline bool operator>(const TlvValue &left, const TlvValue &right)
{
    if (left.dtype() == right.dtype())
    {
        if (left.is_double())
        {
            return left.to_double() > right.to_double();
        }
        else if (left.is_long())
        {
            return left.to_long() > right.to_long();
        }
        else
        {
            return ::strcmp(left.c_str(), right.c_str()) > 0;
        }
    }
    throw std::logic_error("dtype mismatch");
}

inline bool operator==(const TlvValue &left, const TlvValue &right)
{
    if (left.dtype() == right.dtype())
    {
        if (left.is_double())
        {
            return static_cast<int64_t>((left.to_double() - right.to_double()) * 1000000000000L) == 0;
        }
        else if (left.is_long())
        {
            return left.to_long() == right.to_long();
        }
        else
        {
            return ::strcmp(left.c_str(), right.c_str()) == 0;
        }
    }
    return false;
}

inline bool operator!=(const TlvValue &left, const TlvValue &right)
{
    if (left.dtype() == right.dtype())
    {
        if (left.is_double())
        {
            return static_cast<int64_t>((left.to_double() - right.to_double()) * 1000000000000L) != 0;
        }
        else if (left.is_long())
        {
            return left.to_long() != right.to_long();
        }
        else
        {
            return ::strcmp(left.c_str(), right.c_str()) != 0;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

}