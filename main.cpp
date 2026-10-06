#include <iostream>
#include <complex>
#include <vector>
#include <vector>
#include <array>
#include <algorithm>
#include <limits>
#include <cstdint>
#include <numbers>

#include <fstream>

namespace Modem
{
    using cf32 = std::complex<float>;

    static constexpr auto lpf_taps = std::to_array<float>({
        -1.947330676e-04f,  2.198543489e-04f,  5.269785043e-04f,  5.204241583e-04f,
        1.524465056e-04f, -4.025220790e-04f, -8.017481397e-04f, -7.189612557e-04f,
        -8.133437146e-05f,  7.982959778e-04f,  1.334587533e-03f,  1.027191168e-03f,
        -1.344012650e-04f, -1.519549553e-03f, -2.153357738e-03f, -1.361134782e-03f,
        6.409492081e-04f,  2.677278215e-03f,  3.252920799e-03f,  1.592056429e-03f,
        -1.614061999e-03f, -4.379028861e-03f, -4.592739407e-03f, -1.542233694e-03f,
        3.266717650e-03f,  6.737639227e-03f,  6.098877476e-03f,  9.713541702e-04f,
        -5.879111185e-03f, -9.903012874e-03f, -7.670321408e-03f,  4.661519575e-04f,
        9.886609207e-03f,  1.414949951e-02f,  9.188965642e-03f, -3.352076292e-03f,
        -1.613893644e-02f, -2.012928432e-02f, -1.053210537e-02f,  8.950755511e-03f,
        2.680159445e-02f,  2.979358232e-02f,  1.158592407e-02f, -2.122816182e-02f,
        -4.987694038e-02f, -5.169719555e-02f, -1.225830450e-02f,  6.509719860e-02f,
        1.579541572e-01f,  2.333310379e-01f,  2.622764173e-01f,  2.333310379e-01f,
        1.579541572e-01f,  6.509719860e-02f, -1.225830450e-02f, -5.169719555e-02f,
        -4.987694038e-02f, -2.122816182e-02f,  1.158592407e-02f,  2.979358232e-02f,
        2.680159445e-02f,  8.950755511e-03f, -1.053210537e-02f, -2.012928432e-02f,
        -1.613893644e-02f, -3.352076292e-03f,  9.188965642e-03f,  1.414949951e-02f,
        9.886609207e-03f,  4.661519575e-04f, -7.670321408e-03f, -9.903012874e-03f,
        -5.879111185e-03f,  9.713541702e-04f,  6.098877476e-03f,  6.737639227e-03f,
        3.266717650e-03f, -1.542233694e-03f, -4.592739407e-03f, -4.379028861e-03f,
        -1.614061999e-03f,  1.592056429e-03f,  3.252920799e-03f,  2.677278215e-03f,
        6.409492081e-04f, -1.361134782e-03f, -2.153357738e-03f, -1.519549553e-03f,
        -1.344012650e-04f,  1.027191168e-03f,  1.334587533e-03f,  7.982959778e-04f,
        -8.133437146e-05f, -7.189612557e-04f, -8.017481397e-04f, -4.025220790e-04f,
        1.524465056e-04f,  5.204241583e-04f,  5.269785043e-04f,  2.198543489e-04f,
        -1.947330676e-04f,
    });

    static constexpr auto rrc_taps = std::to_array<float>({
        -1.168082527e-03f, -7.260772332e-06f,  1.315159211e-03f,  2.395256019e-03f,
        2.858261914e-03f,  2.475702382e-03f,  1.250769531e-03f, -5.567660150e-04f,
        -2.479860959e-03f, -3.961034522e-03f, -4.509470941e-03f, -3.856343666e-03f,
        -2.062156806e-03f,  4.613477248e-04f,  3.031644573e-03f,  4.867442234e-03f,
        5.313591684e-03f,  4.060018027e-03f,  1.289800020e-03f, -2.296999908e-03f,
        -5.609324639e-03f, -7.428012557e-03f, -6.752693217e-03f, -3.148873765e-03f,
        3.001175010e-03f,  1.042888736e-02f,  1.714145201e-02f,  2.081325352e-02f,
        1.934588370e-02f,  1.148499954e-02f, -2.647484251e-03f, -2.124658788e-02f,
        -4.084928046e-02f, -5.673483457e-02f, -6.366453603e-02f, -5.684555148e-02f,
        -3.294289711e-02f,  9.063952899e-03f,  6.736876285e-02f,  1.373035102e-01f,
        2.118259287e-01f,  2.824727448e-01f,  3.406262356e-01f,  3.788733947e-01f,
        3.922110613e-01f,  3.788733947e-01f,  3.406262356e-01f,  2.824727448e-01f,
        2.118259287e-01f,  1.373035102e-01f,  6.736876285e-02f,  9.063952899e-03f,
        -3.294289711e-02f, -5.684555148e-02f, -6.366453603e-02f, -5.673483457e-02f,
        -4.084928046e-02f, -2.124658788e-02f, -2.647484251e-03f,  1.148499954e-02f,
        1.934588370e-02f,  2.081325352e-02f,  1.714145201e-02f,  1.042888736e-02f,
        3.001175010e-03f, -3.148873765e-03f, -6.752693217e-03f, -7.428012557e-03f,
        -5.609324639e-03f, -2.296999908e-03f,  1.289800020e-03f,  4.060018027e-03f,
        5.313591684e-03f,  4.867442234e-03f,  3.031644573e-03f,  4.613477248e-04f,
        -2.062156806e-03f, -3.856343666e-03f, -4.509470941e-03f, -3.961034522e-03f,
        -2.479860959e-03f, -5.567660150e-04f,  1.250769531e-03f,  2.475702382e-03f,
        2.858261914e-03f,  2.395256019e-03f,  1.315159211e-03f, -7.260772332e-06f,
        -1.168082527e-03f,
    });

    // Group Delay as lpf_delay = (len(lpf_taps) - 1) // 2

    // Implementation from https://wiki.gnuradio.org/index.php/AGC
    class AGC
    {
        public:
            AGC(float reference, float gain, float rate, float min_gain, float max_gain)
            {
                _max_gain = max_gain;
                _min_gain = min_gain;
                _rate = rate;
                _reference = reference;
                _gain = gain;
            }

            void process(const std::vector<cf32> &input, std::vector<cf32> &output)
            {
                output.clear();
                output.reserve(input.size());
                for (std::size_t i = 0; i < input.size(); i++)
                {
                    cf32 sample = input[i] * _gain;
                    output.push_back(sample);
                    float amplitude = std::abs(sample);
                    float difference = _reference - amplitude;
                    float result = difference * _rate;
                    _gain += result;
                    _gain = std::clamp(_gain, _min_gain, _max_gain);
                }
            }
        private:
            float _max_gain;
            float _min_gain;
            float _rate;
            float _reference;
            float _gain;
    };

    // Implementation from https://doi.org/10.1109/TCOM.1975.1092840
    class DDAGC
    {
        public:
            DDAGC(float gain, float rate, float min_gain, float max_gain)
            {
                _max_gain = max_gain;
                _min_gain = min_gain;
                _rate = rate;
                _gain = gain;
            }

            void process(const std::vector<cf32> &input, std::vector<cf32> &output)
            {
                output.clear();
                output.reserve(input.size());
                for (std::size_t i = 0; i < input.size(); i++)
                {
                    cf32 sample = input[i] * _gain;
                    cf32 nearest = _find_nearest(sample);
                    output.push_back(sample);

                    float error_i = std::abs(nearest.real()) - std::abs(sample.real());
                    float error_q = std::abs(nearest.imag()) - std::abs(sample.imag());

                    _gain += _rate * (error_i + error_q);
                    _gain = std::clamp(_gain, _min_gain, _max_gain);
                }
            }
        private:
            float _max_gain;
            float _min_gain;
            float _rate;
            float _gain = 1;

            static constexpr float A = 0.31622776601683794f; // 1/sqrt(10) = Es = 1

            static constexpr std::array<cf32, 16> _points = {{
                {-3*A, -3*A}, {-3*A, -1*A}, {-3*A, +3*A}, {-3*A, +1*A},
                {-1*A, -3*A}, {-1*A, -1*A}, {-1*A, +3*A}, {-1*A, +1*A},
                {+3*A, -3*A}, {+3*A, -1*A}, {+3*A, +3*A}, {+3*A, +1*A},
                {+1*A, -3*A}, {+1*A, -1*A}, {+1*A, +3*A}, {+1*A, +1*A},
            }};

            cf32 _find_nearest(const cf32 &a)
            {
                cf32 closest{0.0f, 0.0f};
                float minimal_distance = std::numeric_limits<float>::max();
                for (std::size_t i = 0; i < _points.size(); i++)
                {
                    cf32 point = _points[i];
                    float distance = std::abs(a - point);
                    if (distance < minimal_distance)
                    {
                        minimal_distance = distance;
                        closest = point;
                    }
                }
                return closest;
            }
    };

    class FIRFilter
    {
        public:
        FIRFilter(const float* filter_taps, std::size_t n)
            : _taps(filter_taps, filter_taps + n), _num_taps(n)
            {
                _history.resize(_num_taps-1);
            }

            void process(const std::vector<cf32> &input, std::vector<cf32> &output)
            {
                std::size_t N = _taps.size();
                std::size_t H = _history.size();

                _working_buffer.clear();
                _working_buffer.reserve(H + input.size());

                // History
                _working_buffer.insert(_working_buffer.end(), _history.begin(), _history.end());
                // Input data
                _working_buffer.insert(_working_buffer.end(), input.begin(), input.end());

                output.clear();
                output.reserve(input.size());

                // y[n]
                for (std::size_t n = H; n < _working_buffer.size(); n++)
                {
                    // h[k] * x[n-k]
                    cf32 dot_product{0.0f, 0.0f};
                    for (std::size_t k = 0; k < N; k++)
                    {
                        dot_product += _taps[k] * _working_buffer[n - k];
                    }
                    output.push_back(dot_product);
                }

                std::copy(_working_buffer.end() - H, _working_buffer.end(), _history.begin());
            }

            void reset()
            {
                std::fill(_history.begin(), _history.end(), cf32{0.0f, 0.0f});
            }

            int group_delay()
            {
                return (_num_taps - 1) / 2;
            }
        private:
            std::vector<float> _taps;
            int _num_taps;
            std::vector<cf32> _history;
            std::vector<cf32>  _working_buffer;
    };


    struct PLLCoefficients
    {
        float K_P  = 0.0f;
        float K_I  = 0.0f;
        float K_D  = 1.0f;       // Gain needs to be computed by slope of S-Curve (simulation)
        float K_0  = 1.0f;
        float zeta = 0.7071f;   

        void calculate(float BnTs)
        {
            float theta_n = (BnTs) / (zeta + 1.0f / (4.0f * zeta));

            K_P = (1.0f / (K_D * K_0)) * ((4.0f * zeta * theta_n) / (1.0f + 2.0f * zeta * theta_n + theta_n * theta_n));
            K_I = (1.0f / (K_D * K_0)) * ((4.0f * theta_n * theta_n) / (1.0f + 2.0f * zeta * theta_n + theta_n * theta_n));
        }
    };

    inline cf32 lerp(const cf32 &a, const cf32 &b, float t)
    {
        return { a.real() + t * (b.real() - a.real()),a.imag() + t * (b.imag() - a.imag()) };
    }

    class GardnerTED
    {
    public:
        GardnerTED(float BnTs, float sps, float kD = 1.0f) : _sps(sps)
        {
            _coeffs.K_D = kD;
            _coeffs.calculate(BnTs);
            _t = sps;
            _t_prev = 0.0f;
        }

        void process(const std::vector<cf32> &input, std::vector<cf32> &output)
        {
            _working_buffer.insert(_working_buffer.end(), input.begin(), input.end());
            output.clear();
            while (_t + 1.0f < static_cast<float>(_working_buffer.size()))
            {
                const cf32 y = interp(_t);
                const cf32 mid = interp(0.5f * (_t_prev + _t));

                float e = (y.real() - _prev_symbol.real()) * mid.real() + (y.imag() - _prev_symbol.imag()) * mid.imag();

                _integral = std::clamp(_integral + _coeffs.K_I * e, -_vmax, _vmax);
                _v = std::clamp(_coeffs.K_P * e + _integral, -_vmax, _vmax);

                output.push_back(y);
                _prev_symbol = y;
                _t_prev = _t;
                _t += _sps / (1.0f + _v);
            }

            const size_t drop = static_cast<size_t>(_t_prev);
            _working_buffer.erase(_working_buffer.begin(), _working_buffer.begin() + drop);
            _t -= static_cast<float>(drop);
            _t_prev -= static_cast<float>(drop);
        }

        void reset()
        {
            _v = 0.0f;
            _integral = 0.0f;
            _prev_symbol = {0.0f, 0.0f};
            _t = _sps;
            _t_prev = 0;
            _working_buffer.clear();
        }

    private:
        cf32 interp(float t) const
        {
            const size_t i = static_cast<size_t>(t);
            return Modem::lerp(_working_buffer[i], _working_buffer[i + 1], t - static_cast<float>(i));
        }

        PLLCoefficients _coeffs;
        float _sps;
        double _t, _t_prev;
        float _v = 0.0f, _integral = 0.0f, _vmax = 0.15f;
        cf32  _prev_symbol{0.0f, 0.0f};
        std::vector<cf32> _working_buffer;
    };

    class DDPLL
    {
    public:
        DDPLL(float BnTs, float kD = 1.0f)
        {
            _coeffs.K_D = kD;
            _coeffs.calculate(BnTs);
        }

        void process(const std::vector<cf32> &input, std::vector<cf32> &output)
        {
            output.clear();
            output.reserve(input.size());
            for (std::size_t i = 0; i < input.size(); i++)
            {
                cf32 current = input[i];
                cf32 derotated = current * std::polar(1.0f, -_phase);
                output.push_back(derotated);

                cf32 nearest_point = _find_nearest(derotated);
                float error = _cross_product(derotated, nearest_point);

                _integral += (_coeffs.K_I * error);

                _phase += _integral + (_coeffs.K_P * error);
                _wrap_phase();

            }
        }

        void reset()
        {
            _integral = 0;
            _phase = 0;
        }

    private:

        PLLCoefficients _coeffs;
        float _integral = 0; // Frequency
        float _phase = 0;

        static constexpr float A = 0.31622776601683794f; // 1/sqrt(10) = Es = 1

        static constexpr std::array<cf32, 16> _points = {{
            {-3*A, -3*A}, {-3*A, -1*A}, {-3*A, +3*A}, {-3*A, +1*A},
            {-1*A, -3*A}, {-1*A, -1*A}, {-1*A, +3*A}, {-1*A, +1*A},
            {+3*A, -3*A}, {+3*A, -1*A}, {+3*A, +3*A}, {+3*A, +1*A},
            {+1*A, -3*A}, {+1*A, -1*A}, {+1*A, +3*A}, {+1*A, +1*A},
        }};

        cf32 _find_nearest(const cf32 &a)
        {
            cf32 closest{0.0f, 0.0f};
            float minimal_distance = std::numeric_limits<float>::max();
            for (std::size_t i = 0; i < _points.size(); i++)
            {
                cf32 point = _points[i];
                float distance = std::abs(a - point);
                if (distance < minimal_distance)
                {
                    minimal_distance = distance;
                    closest = point;
                }
            }
            return closest;
        }

        float _cross_product(const cf32 &a, const cf32 &b)
        {
            return (b.real() * a.imag()) - (b.imag() * a.real());
        }

        void _wrap_phase()
        {
            while (_phase >= 2*std::numbers::pi)
            {
                _phase -= 2*std::numbers::pi;
            }
                while (_phase < 0)
            {
                _phase += 2*std::numbers::pi;
            }
        }
    };

    class BitSlicer
    {
        public:
            BitSlicer() = default;
            void process(const std::vector<cf32> &input, std::vector<uint8_t> &output)
            {
                output.clear();
                output.reserve(input.size());
                for (std::size_t i = 0; i < input.size(); i++)
                {
                    uint8_t bits = _find_nearest(input[i]);

                    output.push_back(bits);
                }
            }    

        private:
            static constexpr float A = 0.31622776601683794f; // 1/sqrt(10) = Es = 1

            static constexpr std::array<cf32, 16> _points = {{
                {-3*A, -3*A}, {-3*A, -1*A}, {-3*A, +3*A}, {-3*A, +1*A},
                {-1*A, -3*A}, {-1*A, -1*A}, {-1*A, +3*A}, {-1*A, +1*A},
                {+3*A, -3*A}, {+3*A, -1*A}, {+3*A, +3*A}, {+3*A, +1*A},
                {+1*A, -3*A}, {+1*A, -1*A}, {+1*A, +3*A}, {+1*A, +1*A},
            }};

            static constexpr std::array<uint8_t, 16> _bits = {{
                0b0000, 0b0001, 0b0010, 0b0011,
                0b0100, 0b0101, 0b0110, 0b0111,
                0b1000, 0b1001, 0b1010, 0b1011,
                0b1100, 0b1101, 0b1110, 0b1111,
            }};

        uint8_t _find_nearest(const cf32 &a)
        {
            uint8_t closest = 0b0000;
            float minimal_distance = std::numeric_limits<float>::max();
            for (std::size_t i = 0; i < _points.size(); i++)
            {
                cf32 point = _points[i];
                float distance = std::abs(a - point);
                if (distance < minimal_distance)
                {
                    minimal_distance = distance;
                    closest = _bits[i];
                }
            }
            return closest;
        }

    };

    struct Packet
    {
        uint32_t sync_word = 0;
        uint16_t length = 0;
        std::vector<uint8_t> data;
        uint16_t crc = 0;
    };

    class FrameSynchronizer
    {
        public:
            FrameSynchronizer() = default;
            void process(const std::vector<uint8_t> &input, std::vector<Packet> &output)
            {
                uint32_t shift_register = 0;
                output.clear();
                for (std::size_t i = 0; i < input.size(); i++)
                {
                    shift_register = (shift_register << 4) | (input[i] & 0x0F);
                    if (_sync_word == shift_register)
                    {
                        std::size_t sync_start = i - 7;
                        auto packet = _create_packet(input, sync_start);
                        if (packet.length != 0)
                        {
                            output.push_back(packet);
                        }
                    }
                }
            }
        private:
            uint32_t _sync_word = 0x1ACFFC1D;

            uint8_t _bits_to_uint8_t(const std::vector<uint8_t>& input, std::size_t i)
            {
                return static_cast<uint8_t>(
                    (input[i] << 4) |
                    (input[i + 1] & 0x0F)
                );
            }

            uint16_t _bits_to_uint16_t(const std::vector<uint8_t>& input, std::size_t i)
            {
                uint16_t high = _bits_to_uint8_t(input, i);
                uint16_t low  = _bits_to_uint8_t(input, i + 2);

                return static_cast<uint16_t>((high << 8) | low);
            }

            Packet _create_packet(const std::vector<uint8_t>& input, std::size_t index)
            {
                Packet p{};

                if (index + 12 > input.size())
                {
                    p.length = 0;
                    return p;
                }

                p.sync_word = _sync_word;
                p.length = _bits_to_uint16_t(input, index + 8);
                if (p.length > 1800)
                {
                    p.length = 0;
                    return p;
                }

                const std::size_t payload_start = index + 12;
                const std::size_t payload_end = payload_start + p.length * 2;
                const std::size_t crc_start = payload_end;
                const std::size_t packet_end = crc_start + 4;

                if (packet_end > input.size())
                {
                    p.length = 0;
                    return p;
                }

                p.data.reserve(p.length);

                for (std::size_t j = 0; j < p.length; ++j)
                {
                    p.data.push_back(_bits_to_uint8_t(input, payload_start + j * 2));
                }

                p.crc = _bits_to_uint16_t(input, crc_start);
                uint16_t crc = _crc16(p.data.data(), p.length);
                if (p.crc != crc)
                {
                    p.length = 0;
                    return p;
                }

                return p;
            }

            // Source - https://stackoverflow.com/a/44947877
            // Posted by sarvankumar_t, modified by community. See post 'Timeline' for change history
            // Retrieved 2026-10-06, License - CC BY-SA 4.0
            // Modified by me

            uint16_t _crc16(const unsigned char* data, size_t length)
            {
                uint16_t crc = 0xFFFF;

                while (length--)
                {
                    crc ^= static_cast<uint16_t>(*data++) << 8;

                    for (int i = 0; i < 8; ++i)
                    {
                        if (crc & 0x8000)
                            crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
                        else
                            crc = static_cast<uint16_t>(crc << 1);
                    }
                }

                return crc;
            }

    };
}


std::vector<Modem::cf32> read_iq_samples()
{
    std::ifstream iq_file;
    iq_file.open("dump.iq", std::ios::in | std::ios::binary | std::ios::ate);
    std::size_t n = iq_file.tellg() / sizeof(Modem::cf32);
    iq_file.seekg(0);
    std::vector<Modem::cf32> buffer(n);

    iq_file.read(reinterpret_cast<char*>(buffer.data()), n * sizeof(Modem::cf32));

    return buffer;

}


inline bool write_iq_file(const std::string& path,
                          const std::complex<float>* data,
                          std::size_t count,
                          bool append = false)
{
    std::FILE* f = std::fopen(path.c_str(), append ? "ab" : "wb");
    if (!f) {
        std::perror(("write_iq_file: " + path).c_str());
        return false;
    }
 
    // std::complex<float> ma podla standardu layout float[2] = {re, im},
    // takze mozeme zapisat cely buffer naraz.
    std::size_t written = std::fwrite(data, sizeof(std::complex<float>), count, f);
    std::fclose(f);
 
    if (written != count) {
        std::fprintf(stderr, "write_iq_file: zapisanych %zu z %zu vzoriek\n",
                     written, count);
        return false;
    }
    return true;
}
 
inline bool write_iq_file(const std::string& path,
                          const std::vector<std::complex<float>>& data,
                          bool append = false)
{
    return write_iq_file(path, data.data(), data.size(), append);
}


void write_video(
    const std::vector<Modem::Packet>& packets,
    const std::string& filename)
{
    std::ofstream file(
        filename,
        std::ios::binary | std::ios::trunc);

    for (const auto& packet : packets)
    {
        file.write(
            reinterpret_cast<const char*>(packet.data.data()),
            packet.data.size());
    }
}
int main()
{
    Modem::FIRFilter low_pass_filter(Modem::lpf_taps.data(), Modem::lpf_taps.size());
    Modem::FIRFilter rrc_filter(Modem::rrc_taps.data(), Modem::rrc_taps.size());
    Modem::DDPLL pll_loop(0.015f, 1.0f);
    Modem::GardnerTED timing_loop(0.005, 8);
    Modem::AGC agc(0.93f, 0.4, 1.15e-5f, 1e-4f, 1e4f);
    Modem::DDAGC dd_agc(1, 0.016, 0.5, 2);
    Modem::BitSlicer slicer;
    Modem::FrameSynchronizer frame_synchronizer;

    std::vector<Modem::cf32> iq_data = read_iq_samples();
    std::vector<Modem::cf32> filtered_data;
    std::vector<Modem::cf32> matched_filtered_data;
    std::vector<Modem::cf32> agc_data;
    std::vector<Modem::cf32> dd_agc_data;
    std::vector<Modem::cf32> time_synchronized_data;
    std::vector<Modem::cf32> phase_synchronized_data;
    std::vector<uint8_t> bits;
    std::vector<Modem::Packet> packets;


    low_pass_filter.process(iq_data, filtered_data);
    rrc_filter.process(filtered_data, matched_filtered_data);
    agc.process(matched_filtered_data, agc_data);

    timing_loop.process(agc_data, time_synchronized_data);
    pll_loop.process(time_synchronized_data, phase_synchronized_data);
    dd_agc.process(phase_synchronized_data, dd_agc_data);
    slicer.process(dd_agc_data, bits);
    frame_synchronizer.process(bits, packets);

    write_iq_file("symbols.cf32", phase_synchronized_data);
    write_video(packets, "capture.ts");

}