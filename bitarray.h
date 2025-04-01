#ifndef BITARRAY_H
#define BITARRAY_H
#include <vector>
#include <string>
#include <climits>
#include <iostream>

using std::string;

template<class IType = size_t> 
class BitArray { 
    class bitproxy;
public: 
    enum { BITS_PER_BLOCK = CHAR_BIT * sizeof(IType) };

    // Object Management 
    explicit BitArray(size_t = 0); 
    explicit BitArray(const string&); 
    BitArray(const BitArray& b) = default;  // Copy constructor 
    BitArray& operator=(const BitArray& b) = default; // Copy assignment 
    BitArray(BitArray&& b) noexcept;   // Move constructor 
    BitArray& operator=(BitArray&& b) noexcept; // Move assignment 
    size_t capacity() const;               // # of bits the current allocation can hold 
    
    // Helper functions
    void assign_bit(size_t pos, bool value); // sets bit in position specified
    bool read_bit(size_t pos) const; // reads the bit from position

    // Mutators 
    BitArray& operator+=(bool);   // Append a bit 
    BitArray& operator+=(const BitArray& b); // Append a BitArray 
    void erase(size_t pos, size_t nbits = 1);  // Remove “nbits” bits at a position 
    void insert(size_t, bool);         // Insert a bit at a position (slide "right") 
    void insert(size_t pos, const BitArray&);  // Insert an entire BitArray object 


    // Bitwise ops 
    bitproxy operator[](size_t); 
    bool operator[](size_t) const; 
    void toggle(size_t);     
    void toggle();      // Toggles all bits 
    BitArray operator~() const; 
    BitArray operator<<(unsigned int) const;   // Shift operators... 
    BitArray operator>>(unsigned int) const; 
    BitArray& operator<<=(unsigned int); 
    BitArray& operator>>=(unsigned int); 
   
    // Extraction ops 
    BitArray slice(size_t pos, size_t count) const;    // Extracts a new sub-array 
     
    // Comparison ops 
    bool operator==(const BitArray&) const; 
    bool operator!=(const BitArray&) const; 
    bool operator<(const BitArray&) const; 
    bool operator<=(const BitArray&) const; 
    bool operator>(const BitArray&) const; 
    bool operator>=(const BitArray&) const; 
     
    // Counting ops 
    size_t size() const;   // Number of bits in use in the vector 
    size_t count() const;    // The number of 1-bits present 
    bool any() const;   // Optimized version of count() > 0 
 
    // Stream I/O (define these in situ—meaning the bodies are inside the class) 
    friend std::ostream& operator<<(std::ostream&, const BitArray&); 
    friend std::istream& operator>>(std::istream&, BitArray&); 
 
    // String conversion 
    string to_string() const;

    friend std::ostream& operator<<(std::ostream& os, const BitArray& b) {
        for (size_t i = 0; i < b.size_; i++) {
            os << b.read_bit(i);
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, BitArray& b) {
        BitArray temp;
    
        char ch;
        while (is >> ch && (ch == '0' || ch == '1')) {
            temp += (ch == '1');
        }
        if (!is.eof()) {
            is.putback(ch);
        }
        
        if (temp.size_ == 0) {
            is.setstate(std::ios::failbit);
        } else {
            b = std::move(temp);
        }
    
        return is;
    }

private:
    std::vector<IType> blocks;
    size_t size_ = 0;

    class bitproxy {
        BitArray<IType>& arr;
        size_t pos;

    public:
        bitproxy(BitArray<IType>& arr, size_t pos) : arr(arr), pos{pos} {}

        bitproxy& operator=(bool value) {
            arr.assign_bit(pos, value);
            return *this;
        }

        bitproxy& operator=(const bitproxy& other) {
            bool value = static_cast<bool>(other);
            arr.assign_bit(pos, value);
            return *this;
        }

        operator bool() const {
            return arr.read_bit(pos);
        }
    };
};

template<class IType>
void BitArray<IType>::assign_bit(size_t pos, bool value) {
    if (pos > size_) {
        throw std::logic_error("BitArray::assign_bit() position out of range");
    }

    size_t block = pos / BITS_PER_BLOCK;
    size_t offset = pos % BITS_PER_BLOCK;

    IType mask = IType(1) << offset;
    if (value) {
        blocks[block] |= mask; // set bit if value == 1
    } else {
        blocks[block] &= ~mask; // clear bit if value == 0
    }
}

template<class IType>
bool BitArray<IType>::read_bit(size_t pos) const {
    if (pos >= size_) {
        throw std::logic_error ("BitArray::read_bit() position out of range");
    }

    size_t block = pos / BITS_PER_BLOCK;
    size_t offset = pos % BITS_PER_BLOCK;

    return (blocks[block] >> offset) & IType(1);
}

template<class IType>
BitArray<IType>::BitArray(size_t n) : size_(n){
    // Constructor
    size_t nblocks = (n + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
    blocks.resize(nblocks, 0);
}

template<class IType>
BitArray<IType>::BitArray(const string& str) {
    size_ = 0;

    // Validate and count bits
    for (char c : str) {
        if (c != '0' && c != '1') {
            throw std::runtime_error("BitArray: Invalid character in string constructor");
        }
    }

    size_ = str.size();
    size_t nblocks = (size_ + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
    blocks.resize(nblocks, 0);

    for (size_t i = 0; i < str.size(); ++i) {
        assign_bit(i, str[i] == '1');
    }
}

template<class IType>
BitArray<IType>::BitArray(BitArray&& b) noexcept : blocks(std::move(b.blocks)), size_(b.size_) {
    // move constructor
    b.size_ = 0;
}

template<class IType>
BitArray<IType>& BitArray<IType>::operator=(BitArray&& b) noexcept {
    // move assignment
    if (this != &b) {
        blocks = std::move(b.blocks);
        size_ = b.size_;
        b.size_ = 0;
    }
    return *this;
}

template<class IType>
size_t BitArray<IType>::capacity() const {
    // returns how many bits total can be stored
    return blocks.size() * BITS_PER_BLOCK;
}

template<class IType>
BitArray<IType>& BitArray<IType>::operator+=(bool bit) {
    // append a bit
    if (size_ == capacity()) {
        // if full add another block
        blocks.push_back(0);
    }

    assign_bit(size_, bit); // set the new bit
    ++size_;
    return *this;
}

template<class IType>
BitArray<IType>& BitArray<IType>::operator+=(const BitArray& b) {
    // append a BitArray
    size_t original_size = b.size_;
    for (size_t i = 0; i < original_size; i++) {
        *this += b.read_bit(i);
    }

    return *this;
}

template<class IType>
void BitArray<IType>::erase(size_t pos, size_t nbits) {
    // remove nbits at position
    if (pos + nbits > size_) {
        throw std::logic_error("BitArray::erase() out of range");
    }

    for (size_t i = pos + nbits; i < size_; i++) {
        assign_bit(i - nbits, read_bit(i));
    }

    size_ -= nbits;
}

template<class IType>
void BitArray<IType>::insert(size_t pos, bool bit) {
    // insert bit at position, shifting right
    if (pos > size_) {
        throw std::logic_error ("BitArray::insert() out of range");
    }

    if (size_ == capacity()) {
        blocks.push_back(0);
    }

    // shift right
    for (size_t i = size_; i > pos; i--) {
        assign_bit(i, read_bit(i - 1));
    }

    assign_bit(pos, bit);
    ++size_;
}

template<class IType>
void BitArray<IType>::insert(size_t pos, const BitArray& b) {
    // insert entire BitArray object
    if (pos > size_) {
        throw std::logic_error("BitArray::insert() out of range");
    }

    size_ += b.size_;

    while (size_ > capacity()) {
        blocks.push_back(0);
    }

    // shift right
    for (size_t i = size_ - b.size_; i-- > pos;) {
        assign_bit(i + b.size_, read_bit(i));
    }

    // copy new bits
    for (size_t i = 0; i < b.size_; i++) {
        assign_bit(pos + i, b.read_bit(i));
    }
}

template<class IType>
bool BitArray<IType>::operator[](size_t pos) const {
    return read_bit(pos);
}

template<class IType>
typename BitArray<IType>::bitproxy BitArray<IType>::operator[](size_t pos) {
    if (pos >= size_)
        throw std::logic_error("BitArray::operator[] out of range");

    return bitproxy(*this, pos);
}

template<class IType>
void BitArray<IType>::toggle(size_t pos) {
    if (pos >= size_) {
        throw std::logic_error("BitArray::toggle() out of range");
    }

    size_t block = pos / BITS_PER_BLOCK;
    size_t offset = pos % BITS_PER_BLOCK;
    blocks[block] ^= (IType(1) << offset);
}

template<class IType>
void BitArray<IType>::toggle() {
    for (size_t i = 0; i < size_; i++) {
        toggle(i);
    }
}

template<class IType>
BitArray<IType> BitArray<IType>::operator~() const {
    BitArray<IType> result(*this);
    result.toggle();
    return result;
}

template<class IType>
BitArray<IType> BitArray<IType>::operator<<(unsigned int n) const {
    BitArray<IType> result(*this);
    result <<= n;
    return result;
}

template<class IType>
BitArray<IType> BitArray<IType>::operator>>(unsigned int n) const {
    BitArray<IType> result(*this);
    result >>= n;
    return result;
}

template<class IType>
BitArray<IType>& BitArray<IType>::operator>>=(unsigned int n) {
    if (n >= size_) {
        for (auto& block : blocks) block = 0;
        return *this;
    }

    for (size_t i = size_ - 1; i + 1 > n; i--) {
        assign_bit(i, read_bit(i - n));
    }

    for (size_t i = 0; i < n; i++) {
        assign_bit(i, 0);
    }

    return *this;
}

template<class IType>
BitArray<IType>& BitArray<IType>::operator<<=(unsigned int n) {
    if (n >= size_) {
        for (auto& block : blocks) block = 0;
        return *this;
    }

    for (size_t i = 0; i + n < size_; i++) {
        assign_bit(i, read_bit(i + n));
    }

    for (size_t i = size_ - n; i < size_; i++) {
        assign_bit(i, 0);
    }

    return *this;
}

template<class IType>
BitArray<IType> BitArray<IType>::slice(size_t pos, size_t count) const {
    // extracts a new subarray
    if (pos + count > size_) {
        throw std::logic_error("BitArray::slice() out of range");
    }

    BitArray<IType> result(count);
    for (size_t i = 0; i < count; i++) {
        result.assign_bit(i, read_bit(pos + i));
    }

    return result;
}

template<class IType>
bool BitArray<IType>::operator==(const BitArray& b) const {
    if (size_ != b.size_) {
        return false;
    }
    for (size_t i = 0; i < size_; i++) {
        if (read_bit(i) != b.read_bit(i)) {
            return false;
        }
    }

    return true;
}

template<class IType>
bool BitArray<IType>::operator!=(const BitArray& b) const {
    return !(*this == b);
}

template<class IType>
bool BitArray<IType>::operator<(const BitArray& b) const {
    size_t min_size = std::min(size_, b.size_);
    for (size_t i = 0; i < min_size; ++i) {
        if (read_bit(i) != b.read_bit(i))
            return !read_bit(i) && b.read_bit(i);
    }
    return size_ < b.size_;
}

template<class IType>
bool BitArray<IType>::operator<=(const BitArray& b) const {
    return (*this < b) || (*this == b);
}

template<class IType>
bool BitArray<IType>::operator>(const BitArray& b) const {
    return !(*this <= b);
}

template<class IType>
bool BitArray<IType>::operator>=(const BitArray& b) const {
    return !(*this < b);
}

template<class IType>
size_t BitArray<IType>::size() const {
    return size_;
}

template<class IType>
size_t BitArray<IType>::count() const {
    size_t cnt = 0;
    for (size_t i = 0; i < size_; ++i)
        if (read_bit(i))
            ++cnt;
    return cnt;
}

template<class IType>
bool BitArray<IType>::any() const {
    return count() > 0;
}

template<class IType>
std::string BitArray<IType>::to_string() const {
    std::string s;
    for (size_t i = 0; i < size_; i++) {
        s += read_bit(i) ? '1' : '0';
    }
    return s;
}

#endif