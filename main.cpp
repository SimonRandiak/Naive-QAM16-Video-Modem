#include <iostream>
#include <complex>
#include <vector>
#include <vector>
#include <array>
#include <algorithm>

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
        float _t, _t_prev;
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

    private:

        PLLCoefficients _coeffs;
        float _integral = 0; // Frequency
        float _phase = 0;

        static constexpr float A = 0.31622776601683794f; // 1/sqrt(10) -> Es = 1

        static constexpr std::array<cf32, 16> _points = {{
            {-3*A, -3*A}, {-3*A, -1*A}, {-3*A, +3*A}, {-3*A, +1*A},
            {-1*A, -3*A}, {-1*A, -1*A}, {-1*A, +3*A}, {-1*A, +1*A},
            {+3*A, -3*A}, {+3*A, -1*A}, {+3*A, +3*A}, {+3*A, +1*A},
            {+1*A, -3*A}, {+1*A, -1*A}, {+1*A, +3*A}, {+1*A, +1*A},
        }};

        cf32 _find_nearest(const cf32 &a)
        {
            cf32 closest{0.0f, 0.0f};
            float minimal_distance = 10;
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

int main()
{
    Modem::FIRFilter low_pass_filter(Modem::lpf_taps.data(), Modem::lpf_taps.size());
    Modem::FIRFilter rrc_filter(Modem::rrc_taps.data(), Modem::rrc_taps.size());
    Modem::DDPLL pll_loop(0.005f, 1.0f);
    Modem::GardnerTED timing_loop(0.005, 8);

    std::vector<Modem::cf32> iq_data = read_iq_samples();
    std::vector<Modem::cf32> filtered_data;
    std::vector<Modem::cf32> matched_filtered_data;
    std::vector<Modem::cf32> time_synchronized_data;
    std::vector<Modem::cf32> phase_synchronized_data;

    low_pass_filter.process(iq_data, filtered_data);
    rrc_filter.process(filtered_data, matched_filtered_data);
    timing_loop.process(matched_filtered_data, time_synchronized_data);
    pll_loop.process(time_synchronized_data, phase_synchronized_data);

    write_iq_file("symbols.cf32", phase_synchronized_data);
    

}