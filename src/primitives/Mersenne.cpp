//
// Created by moriya on 01/10/17.
//

#include "../../include/primitives/Mersenne.hpp"

template <>
TemplateField<ZpMersenneIntElement>::TemplateField(long fieldParam) {

    this->fieldParam = 2147483647;
    this->elementSizeInBytes = 4;//round up to the next uint8_t
    this->elementSizeInBits = 31;

    auto randomKey = prg.generateKey(128);
    prg.setKey(randomKey);

    m_ZERO = new ZpMersenneIntElement(0);
    m_ONE = new ZpMersenneIntElement(1);
}

#ifdef __x86_64__
template <>
TemplateField<ZpMersenneLongElement>::TemplateField(long fieldParam) {

    this->elementSizeInBytes = 8;//round up to the next uint8_t
    this->elementSizeInBits = 61;

    auto randomKey = prg.generateKey(128);
    prg.setKey(randomKey);

    m_ZERO = new ZpMersenneLongElement(0);
    m_ONE = new ZpMersenneLongElement(1);
}

template <>
TemplateField<ZpMersenne127Element>::TemplateField(long fieldParam) {

    ZpMersenne127Element::init();

    this->elementSizeInBytes = 16;//round up to the next uint8_t
    this->elementSizeInBits = 127;

    auto randomKey = prg.generateKey(128);
    prg.setKey(randomKey);

    m_ZERO = new ZpMersenne127Element(0);
    m_ONE = new ZpMersenne127Element(1);


}
#endif

template <>
ZpMersenneIntElement TemplateField<ZpMersenneIntElement>::GetElement(long b) {


    if(b == 1)
    {
        return *m_ONE;
    }
    if(b == 0)
    {
        return *m_ZERO;
    }
    else{
        ZpMersenneIntElement element(b);
        return element;
    }
}

#ifdef __x86_64__
template <>
ZpMersenne127Element TemplateField<ZpMersenne127Element>::GetElement(long b) {


    if(b == 1)
    {
        return *m_ONE;
    }
    if(b == 0)
    {
        return *m_ZERO;
    }
    else{
        ZpMersenne127Element element(b);
        return element;
    }
}

template <>
ZpMersenneLongElement TemplateField<ZpMersenneLongElement>::GetElement(long b) {


    if(b == 1)
    {
        return *m_ONE;
    }
    if(b == 0)
    {
        return *m_ZERO;
    }
    else{
        ZpMersenneLongElement element(b);
        return element;
    }
}
#endif

template <>
void TemplateField<ZpMersenneIntElement>::elementToBytes(uint8_t* elemenetInBytes, ZpMersenneIntElement& element){

    memcpy(elemenetInBytes, (uint8_t*)(&element.elem), 4);
}

#ifdef __x86_64__
template <>
void TemplateField<ZpMersenneLongElement>::elementToBytes(uint8_t* elemenetInBytes, ZpMersenneLongElement& element){

    memcpy(elemenetInBytes, (uint8_t*)(&element.elem), 8);
}

template <>
void TemplateField<ZpMersenne127Element>::elementToBytes(uint8_t* elemenetInBytes, ZpMersenne127Element& element){

    memcpy(elemenetInBytes, (uint8_t*)(&element.elem), 16);
}
#endif

template <>
void TemplateField<ZpMersenneIntElement>::elementVectorToByteVector(vector<ZpMersenneIntElement> &elementVector, vector<uint8_t> &byteVector){

    copy_byte_array_to_byte_vector((uint8_t *)elementVector.data(), elementVector.size()*elementSizeInBytes, byteVector,0);
}

#ifdef __x86_64__
template <>
void TemplateField<ZpMersenneLongElement>::elementVectorToByteVector(vector<ZpMersenneLongElement> &elementVector, vector<uint8_t> &byteVector){

    copy_byte_array_to_byte_vector((uint8_t *)elementVector.data(), elementVector.size()*elementSizeInBytes, byteVector,0);
}

template <>
void TemplateField<ZpMersenne127Element>::elementVectorToByteVector(vector<ZpMersenne127Element> &elementVector, vector<uint8_t> &byteVector){

    copy_byte_array_to_byte_vector((uint8_t *)elementVector.data(), elementVector.size()*elementSizeInBytes, byteVector,0);
}
#endif

template <>
ZpMersenneIntElement TemplateField<ZpMersenneIntElement>::bytesToElement(uint8_t* elemenetInBytes){

    return ZpMersenneIntElement((unsigned int)(*(unsigned int *)elemenetInBytes));
}

#ifdef __x86_64__
template <>
ZpMersenneLongElement TemplateField<ZpMersenneLongElement>::bytesToElement(uint8_t* elemenetInBytes){

    return ZpMersenneLongElement((unsigned long)(*(unsigned long *)elemenetInBytes));
}

template <>
ZpMersenne127Element TemplateField<ZpMersenne127Element>::bytesToElement(uint8_t* elemenetInBytes){

    return ZpMersenne127Element((__uint128_t)(*(__uint128_t *)elemenetInBytes));
}

__uint128_t ZpMersenne127Element::p = 0;
#endif
