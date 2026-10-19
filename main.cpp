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

#include <zmq.hpp>
#include <unsupported/Eigen/FFT>


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


    // This will be based on 4th power frequency estimation
    class FrequencyEstimator
    {
        public:
            FrequencyEstimator(double sampling_frequency)
            {
                _sampling_frequency = sampling_frequency;
            }
            void process(const std::vector<cf32> &input, std::vector<cf32> &output)
            {

                output.clear();
                _apply_offset(input, output);

                if (_freeze == true)
                {
                    return;
                }

                if (_working_buffer.size() >= _acc_N)
                {
                    // Do FFT here and estimate
                    // We have a lot of samples
                    // After that freeze Frequency Estimator

                    _working_buffer.resize(_acc_N);
                    _power4(_working_buffer, _working_buffer);
                    _working_buffer.resize(_FFT_SIZE); // Zero padding
                    std::vector<cf32> spectrum;
                    //spectrum.reserve(_working_buffer.size());

                    _fft.fwd(spectrum, _working_buffer);
                    //std::rotate(spectrum.begin(), spectrum.begin() + FFT_SIZE / 2, spectrum.end());
                    if (_peak_to_mean(spectrum) > 15)
                    {
                        _freeze = true;
                        _offset_hz = _find_offset(spectrum);
                    }
                    _working_buffer.clear();
                }
                else
                {
                    _working_buffer.insert(_working_buffer.end(), input.begin(), input.end());
                }

            }

            void reset()
            {
                _freeze = false;
                _working_buffer.clear();
            }
        private:
            // Frequency resolution Fs/N
            std::vector<cf32> _working_buffer;
            const std::size_t _acc_N = 16'384;
            const std::size_t _FFT_SIZE = 4 * _acc_N;
            double _sampling_frequency;
            bool _freeze = false;
            float _offset_hz = 0;
            float _max_offset = 50'000;
            Eigen::FFT<float> _fft;
            double _phase = 0;

            void _power4(const std::vector<cf32>& input, std::vector<cf32>& output)
            {
                output.resize(input.size());

                for (size_t i = 0; i < input.size(); ++i)
                {
                    const auto x2 = input[i] * input[i];
                    output[i] = x2 * x2;
                }
            }

            double _max_frequency_range()
            {
                // 4 * delta_f < Fs/2
                // delta_f < Fs/8
                // Power of 4 multiplies the frequency by 4
                // We must stay withing Fs/2
                return _sampling_frequency / 8;
            }

            double _find_offset(const std::vector<cf32>& spectrum) const
            {
                const auto M = static_cast<std::ptrdiff_t>(spectrum.size());
                const auto k_bin = static_cast<std::ptrdiff_t>(4.0 * _max_offset * static_cast<double>(M) / _sampling_frequency);

                std::ptrdiff_t best_k = 0;
                float best_power = -1.0f;
                // - means negative frequency so we cover negative and positive frequency offset
                for (std::ptrdiff_t k = -k_bin; k <= k_bin; ++k)
                {
                    // we use property that DFT is periodic -k/N = (N-k)/N
                    const float p = std::norm(spectrum[static_cast<std::size_t>((k + M) % M)]);
                    if (p > best_power)
                    {
                        best_power = p;
                        best_k = k;
                    }
                }

                const double f4 = static_cast<double>(best_k) * _sampling_frequency / static_cast<double>(M);
                return f4 / 4.0;
            }

            void _apply_offset(const std::vector<cf32>& input, std::vector<cf32>& output)
            {
                const double step = -2.0 * std::numbers::pi_v<float> * _offset_hz / _sampling_frequency;
                for (std::size_t i = 0; i < input.size(); i++)
                {
                    cf32 sample = input[i];
                    sample *= cf32(static_cast<float>(std::cos(_phase)), static_cast<float>(std::sin(_phase)));
                    _phase += step;
                    output.push_back(sample);
                    _wrap_phase();
                }
            }

            double _peak_to_mean(const std::vector<cf32>& input)
            {
                float best_power = -1.0f;
                const auto M = static_cast<std::ptrdiff_t>(input.size());
                const auto k_bin = static_cast<std::ptrdiff_t>(4.0 * _max_offset * static_cast<double>(M) / _sampling_frequency);
                double integrator = 0;
                // - means negative frequency so we cover negative and positive frequency offset
                for (std::ptrdiff_t k = -k_bin; k <= k_bin; ++k)
                {
                    // we use property that DFT is periodic -k/N = (N-k)/N
                    const float p = std::norm(input[static_cast<std::size_t>((k + M) % M)]);
                    integrator += p;
                    if (p > best_power)
                    {
                        best_power = p;
                    }
                }

                return 10 * std::log10(best_power / (integrator / ((2 * k_bin) + 1) ));
            }

            void _wrap_phase()
            {
                while (_phase >= 2*std::numbers::pi_v<float>)
                {
                    _phase -= 2*std::numbers::pi_v<float>;
                }
                    while (_phase < 0)
                {
                    _phase += 2*std::numbers::pi_v<float>;
                }
            }

    };

    // Implementation from https://wiki.gnuradio.org/index.php/AGC
    class AGC
    {
        public:
            AGC(float reference, float gain, float rate, float min_gain, float max_gain)
            {
                _max_log_gain = std::log(max_gain);
                _min_log_gain = std::log(min_gain);
                _rate = rate;
                _log_reference = std::log(reference);
                _log_gain = std::log(gain);
                _epsilon = (reference * reference)*1e-10;
            }

            void process(const std::vector<cf32> &input, std::vector<cf32> &output)
            {
                output.clear();
                output.reserve(input.size());
                for (std::size_t i = 0; i < input.size(); i++)
                {
                    cf32 sample = input[i] * std::exp(_log_gain);
                    output.push_back(sample);
                    float log_amplitude = 0.5f*std::log(std::norm(sample) + _epsilon);
                    float difference = _log_reference - log_amplitude;
                    float result = difference * _rate;
                    _log_gain += result;
                    _log_gain = std::clamp(_log_gain, _min_log_gain, _max_log_gain);
                }
            }
        private:
            float _max_log_gain;
            float _min_log_gain;
            float _rate;
            float _log_reference;
            float _log_gain;
            float _epsilon;
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
                    if (_freeze == true)
                    {
                        continue;
                    }
                    _gain += _rate * (error_i + error_q);
                    _gain = std::clamp(_gain, _min_gain, _max_gain);
                }
            }

            void reset()
            {
                _gain = 1;
            }

            void freeze()
            {
                _freeze = true;
            }
            void unfreeze()
            {
                _freeze = false;
            }
        private:
            float _max_gain;
            float _min_gain;
            float _rate;
            float _gain = 1;
            bool _freeze = false;

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

        void setBnTs(float BnTs, float kD = 1.0f)
        {
            _coeffs.K_D = kD;
            _coeffs.calculate(BnTs);
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
        cf32 interp(double t) const
        {
            const size_t i = static_cast<size_t>(t);
            return Modem::lerp(_working_buffer[i], _working_buffer[i + 1], t - static_cast<double>(i));
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
                float error = _cross_product_normalized(derotated, nearest_point);
                // PLL related
                _integral += (_coeffs.K_I * error);

                _phase += _integral + (_coeffs.K_P * error);
                _wrap_phase();

            }
        }

        void setBnTs(float BnTs, float kD = 1.0f)
        {
            _coeffs.K_D = kD;
            _coeffs.calculate(BnTs);
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

        float _cross_product_normalized(const cf32 &a, const cf32 &b)
        {
            float d = std::norm(b);
            return ((b.real() * a.imag()) - (b.imag() * a.real())) / d;
        }

        void _wrap_phase()
        {
            while (_phase >= 2*std::numbers::pi_v<float>)
            {
                _phase -= 2*std::numbers::pi_v<float>;
            }
                while (_phase < 0)
            {
                _phase += 2*std::numbers::pi_v<float>;
            }
        }
    };

    class SyncStateMachine
    {
        public:
            enum class State
            {
                UNLOCKED,
                TRANSITION,
                LOCKED
            };
        
            void process(const std::vector<cf32> &input)
            {
                for (std::size_t i = 0; i < input.size(); i++)
                {
                    cf32 current = input[i];

                    cf32 nearest_point = _find_nearest(current);
                    _evm_error += std::norm(current - nearest_point);
                    _evm_power += std::norm(nearest_point);
                    _evm_count += 1;
                    if (_evm_count == _evm_N)
                    {
                        _evm_percentage = 100 * std::sqrt(_evm_error / _evm_power);
                        _mer_db = 10 * std::log10(_evm_power / _evm_error);
                        _evm_error = 0.0f;
                        _evm_power = 0.0f;
                        _evm_count = 0;

                        if (_state == State::UNLOCKED)
                        {
                            _unlock_mer += _mer_db;
                            _unlock_count += 1;

                            if (_unlock_count == _unlock_N)
                            {
                                float average_unlock_mer = (_unlock_mer / _unlock_count);
                                if (average_unlock_mer >= _T_lock)
                                {
                                    _state = State::TRANSITION;
                                    _previous_state = State::UNLOCKED;
                                }
                                _unlock_count = 0;
                                _unlock_mer = 0;
                            }
                        }
                        else if (_state == State::LOCKED)
                        {
                            _lock_mer += _mer_db;
                            _lock_count += 1;

                            if (_lock_count == _lock_N)
                            {
                                float average_unlock_mer = (_lock_mer / _lock_count);
                                if (average_unlock_mer <= _T_unlock)
                                {
                                    _state = State::TRANSITION;
                                    _previous_state = State::LOCKED;
                                }
                                _lock_count = 0;
                                _lock_mer = 0;
                            }
                        }
                        else if (_state == State::TRANSITION)
                        {
                            if (_transition_count == _transition_N)
                            {
                                _update_loops = true;
                                _transition_count = 0;
                                if (_previous_state == State::UNLOCKED)
                                {
                                    _state = State::LOCKED;
                                    _previous_state = State::LOCKED;
                                }
                                else if (_previous_state == State::LOCKED)
                                {
                                    _state = State::UNLOCKED;
                                    _previous_state = State::UNLOCKED;
                                }
                            }
                            _transition_count += 1;
                        }

                        std::cerr << "[MER dB] " << _mer_db << std::endl;
                        std::cerr << "[EVM Percentage] " << _evm_percentage << std::endl;
                    }
                }
            }

            bool update()
            {
                if (_update_loops == true)
                {
                    _update_loops = false;
                    return true;
                }
                return false;
            }

            State current_state()
            {
                return _state;
            }

        private:
            static constexpr float _T_lock = 15.5;
            static constexpr float _T_unlock = 13.5;

            bool _update_loops = false;


            State _state = State::UNLOCKED;
            State _previous_state = State::UNLOCKED;
            const int _evm_N = 3000;

            const int _unlock_N = 4;
            const int _lock_N = 4;
            const int _transition_N = 6;

            int _lock_count = 0;
            int _unlock_count = 0;

            int _transition_count = 0;

            float _lock_mer = 0;
            float _unlock_mer = 0;

            float _evm_error = 0;
            float _evm_power = 0;

            int _evm_count = 0;

            float _evm_percentage = 0;
            float _mer_db = 0;
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
            using RotationTable = std::array<std::array<uint8_t, 16>, 4>;

            static constexpr uint32_t sync_word = 0x1ACFFC1D;
            static constexpr std::size_t max_length = 1800;

            static constexpr RotationTable FORWARD = []
            {
                auto bits_to_level = [](uint8_t b) -> int
                {
                    switch (b & 0x3)
                    {
                        case 0b00: return -3;
                        case 0b01: return -1;
                        case 0b10: return +3;
                        default:   return +1;
                    }
                };
                auto level_to_bits = [](int level) -> uint8_t
                {
                    switch (level)
                    {
                        case -3: return 0b00;
                        case -1: return 0b01;
                        case +3: return 0b10;
                        default: return 0b11;
                    }
                };
                auto rotate90 = [&](uint8_t n) -> uint8_t
                {
                    const int i = bits_to_level(static_cast<uint8_t>(n >> 2));
                    const int q = bits_to_level(n);
                    return static_cast<uint8_t>((level_to_bits(-q) << 2) | level_to_bits(i));
                };

                RotationTable table{};
                for (uint8_t n = 0; n < 16; ++n)
                {
                    uint8_t r = n;
                    for (std::size_t k = 0; k < 4; ++k)
                    {
                        table[k][n] = r;
                        r = rotate90(r);
                    }
                }
                return table;
            }();

            static constexpr RotationTable INVERSE = []
            {
                RotationTable table{};
                for (std::size_t k = 0; k < 4; ++k)
                    for (uint8_t n = 0; n < 16; ++n)
                        table[k][FORWARD[k][n]] = n;
                return table;
            }();

            static constexpr std::array<uint32_t, 4> SYNC_ROT = []
            {
                std::array<uint32_t, 4> table{};
                for (std::size_t k = 0; k < 4; ++k)
                {
                    uint32_t word = 0;
                    for (int shift = 28; shift >= 0; shift -= 4)
                        word = (word << 4) | FORWARD[k][(sync_word >> shift) & 0xF];
                    table[k] = word;
                }
                return table;
            }();

            void process(const std::vector<uint8_t> &input, std::vector<Packet> &output)
            {
                output.clear();
                _buffer.insert(_buffer.end(), input.begin(), input.end());

                std::size_t pos = 0;

                while (pos + SYNC_NIBBLES <= _buffer.size())
                {
                    const int k = _match_sync(pos);
                    if (k < 0)
                    {
                        ++pos;
                        continue;
                    }

                    if (pos + HEADER_NIBBLES > _buffer.size())
                        break;

                    const uint16_t length = _read_u16(pos + SYNC_NIBBLES, k);
                    if (length == 0 || length > max_length)
                    {
                        ++pos;
                        continue;
                    }

                    const std::size_t payload_start = pos + HEADER_NIBBLES;
                    const std::size_t crc_start = payload_start + 2u * length;
                    const std::size_t packet_end = crc_start + CRC_NIBBLES;

                    if (packet_end > _buffer.size())
                        break;

                    Packet packet;
                    packet.sync_word = sync_word;
                    packet.length = length;
                    packet.data.resize(length);
                    for (std::size_t j = 0; j < length; ++j)
                        packet.data[j] = _read_u8(payload_start + 2 * j, k);
                    packet.crc = _read_u16(crc_start, k);

                    if (packet.crc == _crc16(packet.data.data(), length))
                    {
                        _rotation = k;
                        ++_packets_ok;
                        output.push_back(std::move(packet));
                        pos = packet_end;
                    }
                    else
                    {
                        ++_crc_errors;
                        ++pos;
                    }
                }

                _buffer.erase(_buffer.begin(), _buffer.begin() + static_cast<std::ptrdiff_t>(pos));
            }

            void reset()
            {
                _buffer.clear();
                _rotation = -1;
                _packets_ok = 0;
                _crc_errors = 0;
            }

            void reset_crc_errors()
            {
                _crc_errors = 0;
            }

            int rotation() const { return _rotation; }
            std::size_t packets_ok() const { return _packets_ok; }
            std::size_t crc_errors() const { return _crc_errors; }

        private:
            static constexpr std::size_t SYNC_NIBBLES = 8;
            static constexpr std::size_t HEADER_NIBBLES = SYNC_NIBBLES + 4;
            static constexpr std::size_t CRC_NIBBLES = 4;

            std::vector<uint8_t> _buffer;
            int _rotation = -1;
            std::size_t _packets_ok = 0;
            std::size_t _crc_errors = 0;

            int _match_sync(std::size_t pos) const
            {
                uint32_t word = 0;
                for (std::size_t i = 0; i < SYNC_NIBBLES; ++i)
                    word = (word << 4) | (_buffer[pos + i] & 0x0F);

                for (int k = 0; k < 4; ++k)
                    if (word == SYNC_ROT[k])
                        return k;
                return -1;
            }

            uint8_t _read_u8(std::size_t pos, int k) const
            {
                const auto &inv = INVERSE[k];
                return static_cast<uint8_t>((inv[_buffer[pos] & 0x0F] << 4) |
                                             inv[_buffer[pos + 1] & 0x0F]);
            }

            uint16_t _read_u16(std::size_t pos, int k) const
            {
                return static_cast<uint16_t>((_read_u8(pos, k) << 8) | _read_u8(pos + 2, k));
            }

            static uint16_t _crc16(const uint8_t *data, std::size_t length)
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


void write_video(const std::vector<Modem::Packet>& packets, std::ostream& out)
{
    for (const auto& packet : packets) {
        out.write(reinterpret_cast<const char*>(packet.data.data()),
                  packet.data.size());
        out.flush();
    }
}

void write_video_to_stdout(const std::vector<Modem::Packet>& packets)
{
    for (const auto& packet : packets)
    {
        std::cout.write(
            reinterpret_cast<const char*>(packet.data.data()),
            static_cast<std::streamsize>(packet.data.size()));
    }

    std::cout.flush();
}

int main()
{

    static constexpr float sampling_frequency = 2'400'000;
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::pull);
    sock.connect("tcp://127.0.0.1:5555");
    std::cerr << "[START] src=zmq://127.0.0.1:5555  fs=2400000  rs=300000  sps=8" << std::endl;

    Modem::SyncStateMachine state_machine;
    Modem::FrequencyEstimator coarse_estimator(sampling_frequency);
    Modem::FIRFilter low_pass_filter(Modem::lpf_taps.data(), Modem::lpf_taps.size());
    Modem::FIRFilter rrc_filter(Modem::rrc_taps.data(), Modem::rrc_taps.size());
    Modem::DDPLL pll_loop(0.06f, 1.0f); // kD needs callibration
    Modem::GardnerTED timing_loop(0.005, 8);
    Modem::AGC agc(0.8f, 0.4, 1.15e-5f, 1e-4f, 1e4f);
    Modem::DDAGC dd_agc(1, 0.016, 0.5, 2);
    Modem::BitSlicer slicer;
    Modem::FrameSynchronizer frame_synchronizer;

    std::vector<Modem::cf32> iq_data(4096*5);
    std::vector<Modem::cf32> coarse_frequency_data;
    std::vector<Modem::cf32> filtered_data;
    std::vector<Modem::cf32> matched_filtered_data;
    std::vector<Modem::cf32> agc_data;
    std::vector<Modem::cf32> dd_agc_data;
    std::vector<Modem::cf32> time_synchronized_data;
    std::vector<Modem::cf32> phase_synchronized_data;
    std::vector<uint8_t> bits;
    std::vector<Modem::Packet> packets;


    dd_agc.freeze();

    while (true)
    {
        zmq::message_t msg;
        if (!sock.recv(msg, zmq::recv_flags::none))
        {
            continue;
        }

        const auto* samples = static_cast<const Modem::cf32*>(msg.data());
        const std::size_t n = msg.size() / sizeof(Modem::cf32);
        if (n == 0)
        {
            continue;
        }
        iq_data.assign(samples, samples + n);

        
        low_pass_filter.process(iq_data, filtered_data);
        coarse_estimator.process(filtered_data, coarse_frequency_data);
        rrc_filter.process(coarse_frequency_data, matched_filtered_data);
        agc.process(matched_filtered_data, agc_data);

        timing_loop.process(agc_data, time_synchronized_data);
        pll_loop.process(time_synchronized_data, phase_synchronized_data);
        dd_agc.process(phase_synchronized_data, dd_agc_data);

        state_machine.process(dd_agc_data);

        if (state_machine.update())
        {
            if (state_machine.current_state() == Modem::SyncStateMachine::State::LOCKED)
            {
                dd_agc.unfreeze();
                pll_loop.setBnTs(0.015);
            }
            else if (state_machine.current_state() == Modem::SyncStateMachine::State::UNLOCKED)
            {
                dd_agc.reset();
                dd_agc.freeze();
                pll_loop.setBnTs(0.060);
                coarse_estimator.reset();
                pll_loop.reset();
            }
        }

        slicer.process(dd_agc_data, bits);
        frame_synchronizer.process(bits, packets);
        if (frame_synchronizer.crc_errors() > 20)
        {
            coarse_estimator.reset();
            frame_synchronizer.reset_crc_errors();
        }
        //write_iq_file("symbols.iq", dd_agc_data);
        //write_video(packets, video);
        write_video_to_stdout(packets);
    }
}