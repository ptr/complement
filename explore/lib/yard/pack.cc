#include <yard/pack.h>
#include <assert.h>

namespace yard {

void uuid_to_long_number(const xmt::uuid_type& u, uint8_t* long_number)
{
    for (uint64_t i = 0; i < 8; ++i)
    {
        if (i != 7)
        {
            long_number[i] = (u.u.l[1] % (1ll << (8 * (i + 1)))) / (1ll << 8 * i);
            long_number[i + 8] = (u.u.l[0] % (1ll << (8 * (i + 1)))) / (1ll << 8 * i);
        }
        else
        {
            long_number[i] = u.u.l[1] / (1ll << 8 * i);
            long_number[i + 8] = u.u.l[0] / (1ll << 8 * i);
        }
    }
}

void long_number_to_uuid(const uint8_t* long_number, xmt::uuid_type& u)
{
    u.u.l[0] = u.u.l[1] = 0;
    for (uint64_t i = 0; i < 8; ++i)
    {
        u.u.l[1] += long_number[i] * (1ll << 8*i);
        u.u.l[0] += long_number[8 + i] * (1ll << 8*i);
    }
}

void uuid_packer_exp::unpack( std::istream& s, xmt::uuid_type& u )
{
    uint8_t number[16];
    for (int i = 0; i < 16; ++i)
        number[i] = 0;

    uint8_t exp;

    s.read((char*)&exp, sizeof(uint8_t));
    assert(exp <= 16);

    if (exp < 16)
        s.read((char*)(number + exp), 16 - exp);

    long_number_to_uuid(number, u);
}

void uuid_packer_exp::pack( std::ostream& s, const xmt::uuid_type& u )
{
    uint8_t number[16];
    uuid_to_long_number(u, number);

    uint64_t x1 = 0;
    uint64_t x0 = 0;
    for (uint64_t i = 0; i < 8; ++i)
    {
        x1 += number[i] * (1ll << 8*i);
        x0 += number[8 + i] * (1ll << 8*i);
    }

    assert(x1 == u.u.l[1]);
    assert(x0 == u.u.l[0]);

    uint8_t exp = 0;
    for (long i = 0 ; i < 16; ++i)
        if (number[i] == 0)
            ++exp;
        else
            break;

    assert(exp <= 16);

    s.write((char*)&exp, sizeof(uint8_t));
    if (exp < 16)
        s.write((char*)(number + exp), 16 - exp);
}

__FIT_DECLSPEC void varint_packer::unpack( std::istream& s, uint32_t& num )
{
    uint32_t result = 0;
    char current = 0;
    int count = 0;
    do
    {
        assert(count <= 4);

        s.get(current);
        result += (current & 127) << (7 * count);
        ++count;
    } while (current < 0);
    num = result;
}

__FIT_DECLSPEC void varint_packer::pack( std::ostream& s, const uint32_t& num )
{
    uint32_t num_to_pack = num;
    do
    {
        char current = num_to_pack % 128;
        num_to_pack = num_to_pack / 128;
        if (num_to_pack != 0)
            current += 128;

        s.put(current);
    }
    while (num_to_pack != 0);
}

}
