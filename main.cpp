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
#include <chrono>
#include <bit>

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
            enum class State
            {
                FREQUENCY_ESTIMATION,
                TRACKING_SIGNAL,
            };

            FrequencyEstimator(double sampling_frequency)
            {
                _sampling_frequency = sampling_frequency;
            }
            void process(const std::vector<cf32> &input, std::vector<cf32> &output)
            {

                output.clear();

                if (_state == State::FREQUENCY_ESTIMATION)
                {
                    _estimate_frequency(input);
                }

                else if (_state == State::TRACKING_SIGNAL)
                {
                    _estimate_signal_presence(input);
                }

                _apply_offset(input, output);
            }

            bool signal_is_present()
            {
                return _signal;
            }

            void reset()
            {
                _state = State::FREQUENCY_ESTIMATION;
                _signal = false;
                _working_buffer.clear();
                _peak_to_mean = 0;
            }

            double peak_to_mean()
            {
                return _peak_to_mean;
            }

            bool new_estimate()
            {
                if (_new_estimate_available == true)
                {
                    _new_estimate_available = false;
                    return true;
                }
                return false;
            }
            
        private:
            State _state = State::FREQUENCY_ESTIMATION;
            // Frequency resolution Fs/N
            std::vector<cf32> _working_buffer;
            const std::size_t _acc_N = 16'384;
            const std::size_t _FFT_SIZE = 4 * _acc_N;
            const float _freq_estimation_T = 15;
            const float _signal_estimation_T = 13;
            bool _signal = false;
            double _sampling_frequency;
            float _offset_hz = 0;
            float _max_offset = 50'000;
            Eigen::FFT<float> _fft;
            double _phase = 0;
            double _peak_to_mean = 0;
            bool _new_estimate_available = false;


            void _estimate_frequency(const std::vector<cf32> &input)
            {
                // TODO make it so working_buffer is always power of 2
                _working_buffer.insert(_working_buffer.end(), input.begin(), input.end());
                if (_working_buffer.size() >= _acc_N)
                {
                    // Do FFT here and estimate
                    // We have a lot of samples
                    // After that freeze Frequency Estimator

                    _working_buffer.resize(_round_to_power_to_2(_working_buffer));
                    _power4(_working_buffer, _working_buffer);
                    _working_buffer.resize(_FFT_SIZE); // Zero padding
                    std::vector<cf32> spectrum;
                    //spectrum.reserve(_working_buffer.size());

                    _fft.fwd(spectrum, _working_buffer);
                    //std::rotate(spectrum.begin(), spectrum.begin() + FFT_SIZE / 2, spectrum.end());
                    
                    _peak_to_mean = _calculate_peak_to_mean(spectrum);
                    
                    if (_peak_to_mean > _freq_estimation_T)
                    {
                        _offset_hz = _find_offset(spectrum);
                        _signal = true;
                        _state = State::TRACKING_SIGNAL;
                        _new_estimate_available = true;
                    }
                    _working_buffer.clear();
                }
            }


            void _estimate_signal_presence(const std::vector<cf32> &input)
            {
                _working_buffer.insert(_working_buffer.end(), input.begin(), input.end());
                if (_working_buffer.size() >= _acc_N)
                {
                    _working_buffer.resize(_round_to_power_to_2(_working_buffer));
                    _power4(_working_buffer, _working_buffer);
                    std::vector<cf32> spectrum;

                    _fft.fwd(spectrum, _working_buffer);
                    _peak_to_mean = _calculate_peak_to_mean(spectrum);
                    if (_peak_to_mean < _signal_estimation_T)
                    {
                        _signal = false;
                    }
                    else
                    {
                        _signal = true;
                    }
                    _working_buffer.clear();
                }

            }

            std::size_t _round_to_power_to_2(const std::vector<cf32> &input)
            {
                std::size_t n = 1;

                while (n < input.size())
                    n *= 2;

                return n;
            }

            void _power4(const std::vector<cf32>& input, std::vector<cf32>& output)
            {
                output.resize(input.size());

                for (size_t i = 0; i < input.size(); ++i)
                {
                    const auto x2 = input[i] * input[i];
                    output[i] = x2 * x2;
                }
            }

            float _max_frequency_range()
            {
                // 4 * delta_f < Fs/2
                // delta_f < Fs/8
                // Power of 4 multiplies the frequency by 4
                // We must stay withing Fs/2
                return _sampling_frequency / 8;
            }

            float _find_offset(const std::vector<cf32>& spectrum) const
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

                const float f4 = static_cast<double>(best_k) * _sampling_frequency / static_cast<double>(M);
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

            double _calculate_peak_to_mean(const std::vector<cf32>& input)
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

    class QAM16Constellation
    {
        public:
            static constexpr std::size_t size = 16;
            static constexpr double Es = 1.0f;
            static constexpr std::size_t bits_per_symbol = 4;
            static constexpr double Eb = 1.0f / bits_per_symbol;

            static constexpr cf32 slice(const cf32 &a)
            {
                return {_slice(a.real()), _slice(a.imag())};
            }

            static constexpr const std::array<cf32, 16> &points()
            {
                return _points;
            }

        private:
            static constexpr float A = 0.31622776601683794f; // 1/sqrt(10) = Es = 1

            static constexpr std::array<cf32, 16> _points = {{
                {-3*A, -3*A}, {-3*A, -1*A}, {-3*A, +3*A}, {-3*A, +1*A},
                {-1*A, -3*A}, {-1*A, -1*A}, {-1*A, +3*A}, {-1*A, +1*A},
                {+3*A, -3*A}, {+3*A, -1*A}, {+3*A, +3*A}, {+3*A, +1*A},
                {+1*A, -3*A}, {+1*A, -1*A}, {+1*A, +3*A}, {+1*A, +1*A},
            }};
            static constexpr float _slice(float x)
            {
                if (x < -2.0f * A)
                {
                    return -3.0f * A;
                }
                if (x < 0.0f)
                {
                    return -1.0f * A;
                }
                if (x < 2.0f * A)
                {
                    return 1.0f * A;
                }

                return 3.0f * A;
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
                _previous_log_gain = _log_gain;
                _acc_N = 0;
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
                    if (_freeze == true)
                    {
                        continue;
                    }
                    _acc_N += 1;
                    _log_gain += result;
                    _log_gain = std::clamp(_log_gain, _min_log_gain, _max_log_gain);

                    if (_acc_N >= 16'384*2) // magic number for now nneds proper naming
                    {
                        if (_gain_storing == true)
                        {
                            _previous_log_gain = _log_gain;
                        }
                        _acc_N = 0;
                    }
                }
            }

            void freeze()
            {
                _freeze = true;
                _log_gain = _previous_log_gain;
                _acc_N = 0;
            }

            void unfreeze()
            {
                _freeze = false;
            }

            void enable_storing_gain()
            {
                _gain_storing = true;
            }

            void disable_storing_gain()
            {
                _gain_storing = false;
            }
        private:
            float _max_log_gain;
            float _min_log_gain;
            float _rate;
            float _log_reference;
            float _log_gain;
            float _epsilon;
            float _previous_log_gain;
            int _acc_N;
            bool _freeze = false;
            bool _gain_storing = false;
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
                    cf32 z = QAM16Constellation::slice(sample);

                    output.push_back(sample);

                    float error_i = std::abs(z.real()) - std::abs(sample.real());
                    float error_q = std::abs(z.imag()) - std::abs(sample.imag());
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
                output.push_back(y);
                const cf32 mid = interp(0.5f * (_t_prev + _t));
                if (_freeze == true)
                {
                    _prev_symbol = y;
                    _t_prev = _t;
                    _t += _sps / (1.0f + _v);
                    continue;
                }
                float e = (y.real() - _prev_symbol.real()) * mid.real() + (y.imag() - _prev_symbol.imag()) * mid.imag();

                _integral = std::clamp(_integral + _coeffs.K_I * e, -_vmax, _vmax);
                _v = std::clamp(_coeffs.K_P * e + _integral, -_vmax, _vmax);

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

        void freeze()
        {
            _freeze = true;
        }

        void unfreeze()
        {
            _freeze = false;
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
        bool _freeze = false;
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
                if (_freeze == true)
                {
                    _phase += _integral;
                    _wrap_phase();
                    continue;
                }

                cf32 z = QAM16Constellation::slice(derotated);
                float error = _cross_product_normalized(derotated, z);
                // PLL related
                _integral += (_coeffs.K_I * error);

                _integral = std::clamp(_integral, -_frequency_clamp, _frequency_clamp);

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

        void freeze()
        {
            _freeze = true;
        }

        void unfreeze()
        {
            _freeze = false;
        }

    private:

        PLLCoefficients _coeffs;
        float _integral = 0; // Frequency
        float _phase = 0;
        // 215 Hz is doppler frequency with 40 km/h + 5000Hz reserve for drift
        const float _frequency_clamp = 2*std::numbers::pi_v<float> * ((215.0f+5000.0f) /  300'000.0f);
        bool _freeze = false;

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

    class ConstellationAnalyzer
    {
        public:
            void process(const std::vector<cf32> &input)
            {
                for (std::size_t i = 0; i < input.size(); i++)
                {
                    cf32 current = input[i];

                    cf32 z = QAM16Constellation::slice(current);
                    _evm_error += std::norm(current - z);
                    _evm_power += std::norm(z);
                    _evm_count += 1;
                    if (_evm_count == _evm_N)
                    {
                        _new_metrics_avaliable = true;
                        _evm_percentage = 100 * std::sqrt(_evm_error / _evm_power);
                        _mer_db = 10 * std::log10(_evm_power / _evm_error);
                        _evm_error = 0.0f;
                        _evm_power = 0.0f;
                        _evm_count = 0;
                    }
                }
            }

            double evm_percentage()
            {
                return _evm_percentage;
            }

            double mer_db()
            {
                return _mer_db;
            }

            void reset()
            {
                _evm_error = 0.0f;
                _evm_power = 0.0f;
                _evm_count = 0;
            }

            bool new_metrics()
            {
                if (_new_metrics_avaliable == true)
                {
                    _new_metrics_avaliable = false;
                    return true;
                }
                return false;
            }

        private:
            float _evm_error = 0;
            float _evm_power = 0;

            int _evm_count = 0;

            float _evm_percentage = 0;
            float _mer_db = 0;
            float _evm_N = 2000;
            bool _new_metrics_avaliable = false;
    };

    class ModemStateMachine
    {
        public:

            ModemStateMachine()
            {
                _task.freeze_dd_agc = true;
                _task.narrow_loop = false;
                _task.freeze_dd_pll = false;
                _task.freeze_gardner = false;
                _task.freeze_agc = false;
                _task.allow_update_agc_previous_gain = false; 
                
                _event.reset_frequency_estimator = true;
                _event.reset_loops = true;
            }
            struct task
            {
                bool freeze_agc =                               false;
                bool freeze_dd_pll =                            false;
                bool freeze_gardner =                           false;
                bool freeze_dd_agc =                            false;
                bool narrow_loop =                              false;
                bool allow_update_agc_previous_gain =           false;
            };

            struct event
            {
                bool reset_frequency_estimator =    false;
                bool reset_loops =                  false;
                bool reset_mer =                    false;

                void reset()
                {
                    reset_frequency_estimator = false;
                    reset_loops = false;
                    reset_mer = false;
                }
            };

            enum class State
            {
                UNLOCKED,
                HOLD,
                LOCKED,
            };
        
            void update_mer(double mer)
            {
                _fresh_mer = true;
                _mer = mer;
            }
            void update_peak_to_mean(double peak_to_mean)
            {
                _peak_to_mean = peak_to_mean;
            }

            void update_samples_in_sampling_frequency(uint64_t sample_count)
            {
                _count_of_time_samples_in_sampling_frequency += sample_count;
            }

            void update()
            {

                bool signal_present = _peak_to_mean > _signal_presence_threshold_db;

                if (_fresh_frequency_estimate == true)
                {
                    _event.reset_loops = true;
                    _event.reset_mer = true;
                    _count_of_time_samples_in_sampling_frequency = 0;
                }
                switch(_current_state)
                {
                    case State::UNLOCKED:
                        if (_mer > _T_lock && _fresh_mer == true)
                        {
                            _previous_state = _current_state;
                            _current_state = State::LOCKED;
                            _task.freeze_dd_agc = false;
                            _task.freeze_dd_pll = false;
                            _task.freeze_gardner = false;
                            _task.narrow_loop = true;
                            _task.freeze_agc = false;
                            _task.allow_update_agc_previous_gain = true;

                            _event.reset_frequency_estimator = false;

                            _event.reset_mer = false;

                            _count_of_time_samples_in_sampling_frequency = 0;
                        }
                        else if (_count_of_time_samples_in_sampling_frequency > _reacquisition_period_samples)
                        {
                            _event.reset_frequency_estimator = true;
                            _count_of_time_samples_in_sampling_frequency = 0;
                        }
                        if (_peak_to_mean > _signal_presence_threshold_db)
                        {
                            _task.freeze_agc = false;
                        }
                        else
                        {
                            _task.freeze_agc = true;
                        }
                        break;
                    case State::LOCKED:
                        // Peak to mean is evaluation first (Frequency estimator is a first block in DSP chain) and mer will drop anyway too
                        if (_peak_to_mean < _signal_presence_threshold_db)
                        {
                            _previous_state = _current_state;
                            _current_state = State::HOLD;
                            _task.freeze_agc = true;
                            _task.freeze_dd_pll = true;
                            _task.freeze_gardner = true;
                            _task.freeze_dd_agc = true;
                            _task.narrow_loop = true;
                            _task.allow_update_agc_previous_gain = false;

                            _event.reset_frequency_estimator = false;
                            _event.reset_loops = false;


                            _count_of_time_samples_in_sampling_frequency = 0;
                        }
                        else if (_mer < _T_unlock && _fresh_mer == true)
                        {
                            _previous_state = _current_state;
                            _current_state = State::HOLD;
                            _task.freeze_dd_agc = true;
                            _task.narrow_loop = false;
                            _task.freeze_agc = true;
                            _task.freeze_gardner = true;
                            _task.allow_update_agc_previous_gain = false;

                            _event.reset_frequency_estimator = false;
                            _event.reset_loops = false;

                            _count_of_time_samples_in_sampling_frequency = 0;
                        }
                        break;
                    case State::HOLD:
                        if (signal_present == true && _signal_was_present == false)
                        {
                            _event.reset_mer = true;
                            _fresh_mer = false;
                        }
                        if (_peak_to_mean > _signal_presence_threshold_db)
                        {
                            if (_mer > _T_lock && _fresh_mer == true)
                            {
                                _previous_state = _current_state;
                                _current_state = State::LOCKED;
                                _task.freeze_agc = false;
                                _task.freeze_dd_agc = false;
                                _task.narrow_loop = true;
                                _task.freeze_dd_pll = false;
                                _task.freeze_gardner = false;
                                _task.allow_update_agc_previous_gain = true; 

                                _event.reset_frequency_estimator = false;

                                _count_of_time_samples_in_sampling_frequency = 0;
                            }
                            else if (_mer < _T_unlock && _fresh_mer == true && signal_present)
                            {
                                _previous_state = _current_state;
                                _current_state = State::UNLOCKED;
                                _task.freeze_dd_agc = true;
                                _task.narrow_loop = false;
                                _task.freeze_dd_pll = false;
                                _task.freeze_gardner = false;
                                _task.freeze_agc = false;
                                _task.allow_update_agc_previous_gain = false;  

                                _event.reset_frequency_estimator = true;
                                _event.reset_loops = true;

                                _count_of_time_samples_in_sampling_frequency = 0;              
                            }
                        }
                        if (_count_of_time_samples_in_sampling_frequency > _time_samples_in_sampling_frequency_to_switch_to_unlock)
                        {
                            _previous_state = _current_state;
                            _current_state = State::UNLOCKED;
                            _task.freeze_dd_agc = true;
                            _task.narrow_loop = false;
                            _task.freeze_dd_pll = false;
                            _task.freeze_gardner = false;
                            _task.freeze_agc = false;
                            _task.allow_update_agc_previous_gain = false; 
                            
                            _event.reset_frequency_estimator = true;
                            _event.reset_loops = true;

                            _count_of_time_samples_in_sampling_frequency = 0;
                        }
                        break;
                }
                _signal_was_present = signal_present;
                _fresh_mer = false;
                _fresh_frequency_estimate = false;
            }

            void clear_events()
            {
                _event.reset();
            }

            State current_state()
            {
                return _current_state;
            }

            task get_task()
            {
                return _task;
            }

            event get_event()
            {
                return _event;
            }
            void update_frequency_estimate()
            {
                _fresh_frequency_estimate = true;
            }

        private:
            task _task;
            event _event;

            // UNLOCKED state

            static constexpr double _T_lock = 15.5;
            static constexpr double _T_unlock = 13.5;

            // 2 seconds * Sampling rate
            static constexpr uint64_t _time_samples_in_sampling_frequency_to_switch_to_unlock = 2 * 2'400'000;
            static constexpr uint64_t _reacquisition_period_samples = 2 * 2'400'000;
            
            static constexpr float _signal_presence_threshold_db = 13;

            State _current_state = State::UNLOCKED;
            State _previous_state = State::UNLOCKED;


            double _mer = 0;
            double _peak_to_mean = 0;
            uint64_t _count_of_time_samples_in_sampling_frequency = 0;

            bool _fresh_mer = false;
            bool _fresh_frequency_estimate = false;
            bool _signal_was_present = false;

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

    class ccsdsLfsr
    {
        public:
            uint8_t next_bit()
            {

                uint8_t out = _state & 1;

                uint8_t new_b = 0;

                for (uint8_t i = 0; i < 8; i++)
                {
                    if (_taps & (1 << i))
                    {
                        new_b ^= (_state >> i) & 1;
                    }
                }
                _state = (_state >> 1) | (new_b << 7);

                return out;
            }
            uint8_t next_byte()
            {
                uint8_t byte = 0x00;
                for (uint8_t i = 0; i < 8; i++)
                {
                byte = (byte | next_bit() << (7 - i));
                }

                return byte;
            }

            void reset()
            {
                _state = 0xFF;
            }
        private:
            uint8_t _state = 0xFF;
            uint8_t _taps = 0xA9;

    };

    class Scrambler
    {
        public:
            Scrambler()
            {
                for (std::size_t i = 0; i < 256; i++)
                {
                    _table[i] = _lfsr.next_byte();
                }
            }

            uint8_t prbs_byte(std::size_t index)
            {
                return _table[index % 255];
            }

            void scramble(const std::vector<uint8_t> &input, std::vector<uint8_t> &output, std::size_t offset)
            {
                output.clear();
                output.reserve(input.size());

                for (std::size_t i = 0; i < input.size(); i++)
                {
                    output.push_back(input[i] ^ _table[(offset + i) % 255]);
                }
            }
        private:
            ccsdsLfsr _lfsr;
            std::array<uint8_t, 256> _table;
    };

    inline uint8_t bit_parity(uint32_t x)
    {
        return std::popcount(x) & 1u;
    }

    inline uint8_t hamming_distance(uint8_t a, uint8_t b)
    {
        return std::popcount(static_cast<uint8_t>(a ^ b));
    }

    class K7Encoder
    {
        public:
            struct encoder_t
            {
                uint8_t a1 = 0;
                uint8_t a2 = 0;
                uint8_t reg = 0x00;
                uint8_t state = 0x00;
            };

            K7Encoder(uint8_t state)
            {
                _state = state;
            }

            encoder_t process(uint8_t bit)
            {
                bit = bit & 1;
                _register = ((_state << 1) | bit);

                uint8_t a1 = bit_parity(_register & _G1);
                uint8_t a2 = bit_parity(_register & _G2);

                _state = _register & 0x3F;

                encoder_t e;
                e.a1 = a1;
                e.a2 = a2;
                e.reg = _register;
                e.state = _state;

                return e;
            }
        private:
            uint8_t _G1 = 0x4F;
            uint8_t _G2 = 0x6D;
            uint8_t _register = 0x00;
            uint8_t _state = 0x00;
    };

    class ViterbiDecoder
    {
        public:
            ViterbiDecoder()
            {
                for (int i = 0; i < _NUM_STATES; i++)
                {
                    for (uint8_t bit = 0; bit < 2; bit++)
                    {
                        auto t = K7Encoder(i);
                        K7Encoder::encoder_t stats = t.process(bit);
                        _table[i][bit] = (stats.a1 << 1) | stats.a2;
                    }
                }
                reset();
            }

            // received_pair = (a1 << 1) | a2
            void acs(uint8_t received_pair)
            {
                _pm_new.fill(1'000'000);
                uint64_t d = 0;
                // We don't know the state we came from so we need to check all states and find the best one
                for (uint8_t i = 0; i < 64; i++)
                {

                    uint8_t bit = i & 0x01;
                    // from which state I came two possiblities either 1 or 0
                    uint8_t possible_incoming_state_1 = (i >> 1);
                    uint8_t possible_incoming_state_2 = (i >> 1) | 0b100000;

                    uint8_t bits_1 = _table[possible_incoming_state_1][bit];
                    uint8_t bits_2 = _table[possible_incoming_state_2][bit];

                    int metric_1 = _pm_old[possible_incoming_state_1] + hamming_distance(bits_1, received_pair);
                    int metric_2 = _pm_old[possible_incoming_state_2] + hamming_distance(bits_2, received_pair);

                    if (metric_2 < metric_1)
                    {
                        _pm_new[i] = metric_2;
                        d |= uint64_t(1) << i;
                    }
                    else
                    {
                        _pm_new[i] = metric_1;
                    }

                }
                _decision.push_back(d);
                std::swap(_pm_old, _pm_new);
            }

            void reset()
            {
                _pm_old.fill(1'000'000);
                _pm_old[0] = 0;
                _decision.clear();
            }

            std::vector<uint8_t> traceback()
            {
                std::vector<uint8_t> bits;
                bits.reserve(_decision.size());
                // Because of trailing zeores we are sure we are starting from 00 state at the end
                uint64_t state = 0x00;
                for (int64_t i = _decision.size() - 1; i >= 0; i--)
                {
                    uint8_t bit = (state & 1);
                    bits.push_back(bit);
                    // Get bit from last register before shift
                    uint64_t bit_state = (_decision[i] >> state) & 1;
                    // create new state because bit_state was last and after shift it got removed it needs to go first again
                    // bit_state u[t-6] ... u[t-1]
                    state = ((bit_state << 5) | state >> 1);
                }
                std::reverse(bits.begin(), bits.end());
                return bits;
            }

            int path_metric() const
            {
                return _pm_old[0];
            }

            void decode_bytes(const uint8_t* coded, std::size_t coded_size, std::vector<uint8_t>& out)
            {
                reset();
                for (std::size_t i = 0; i < coded_size; i++)
                {
                    for (int shift = 6; shift >= 0; shift -= 2)
                    {
                        acs((coded[i] >> shift) & 0x03);
                    }
                }

                std::vector<uint8_t> bits = traceback();
                const std::size_t n_bytes = bits.size() / 8;

                out.clear();
                for (std::size_t b = 0; b + 1 < n_bytes; b++)
                {
                    uint8_t byte = 0;
                    for (int k = 0; k < 8; k++)
                    {
                        byte = static_cast<uint8_t>((byte << 1) | bits[8 * b + k]);
                    }
                    out.push_back(byte);
                }
            }

        private:
        static constexpr int _K = 7;
        static constexpr int _NUM_STATES = 1 << (_K - 1);
        std::array<std::array<uint8_t, 2>, 64> _table;
        std::array<int, 64> _pm_old{};
        std::array<int, 64> _pm_new{};
        std::vector<uint64_t> _decision;
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

                    // Hlavicka: kodovany blok (length + tail) -> 2 bajty
                    _read_coded(pos + SYNC_NIBBLES, HEADER_CODED_BYTES, k);
                    _viterbi.decode_bytes(_coded.data(), _coded.size(), _decoded);
                    const uint16_t length = static_cast<uint16_t>(
                        ((_decoded[0] ^ _scrambler.prbs_byte(0)) << 8) |
                         (_decoded[1] ^ _scrambler.prbs_byte(1)));
                    if (length == 0 || length > max_length)
                    {
                        ++pos;
                        continue;
                    }

                    // Payload + CRC + tail: kodovany blok -> length + 2 bajty
                    const std::size_t payload_start = pos + HEADER_NIBBLES;
                    const std::size_t payload_coded_bytes = 2u * (length + 3u);
                    const std::size_t packet_end = payload_start + 2u * payload_coded_bytes;

                    if (packet_end > _buffer.size())
                        break;

                    _read_coded(payload_start, payload_coded_bytes, k);
                    _viterbi.decode_bytes(_coded.data(), _coded.size(), _decoded);

                    Packet packet;
                    packet.sync_word = sync_word;
                    packet.length = length;
                    packet.data.resize(length);
                    for (std::size_t j = 0; j < length; ++j)
                        packet.data[j] = _decoded[j] ^ _scrambler.prbs_byte(2 + j);
                    packet.crc = static_cast<uint16_t>(
                        ((_decoded[length] ^ _scrambler.prbs_byte(2 + length)) << 8) |
                         (_decoded[length + 1] ^ _scrambler.prbs_byte(3 + length)));

                    if (packet.crc == _crc16(packet.data.data(), length))
                    {
                        _rotation = k;
                        ++_packets_ok;
                        _corrected_bits += static_cast<std::size_t>(_viterbi.path_metric());
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
                _corrected_bits = 0;
            }

            void reset_crc_errors()
            {
                _crc_errors = 0;
            }

            int rotation() const { return _rotation; }
            std::size_t packets_ok() const { return _packets_ok; }
            std::size_t crc_errors() const { return _crc_errors; }
            // Kolko kodovanych bitov Viterbi opravil v dobrych paketoch (od resetu)
            std::size_t corrected_bits() const { return _corrected_bits; }

        private:
            static constexpr std::size_t SYNC_NIBBLES = 8;
            static constexpr int SYNC_MAX_BIT_ERRORS = 4;
            // Hlavicka: (2 bajty length + 1 tail bajt) * 2 = 6 kodovanych bajtov = 12 nibblov
            static constexpr std::size_t HEADER_CODED_BYTES = 2 * (2 + 1);
            static constexpr std::size_t HEADER_NIBBLES = SYNC_NIBBLES + 2 * HEADER_CODED_BYTES;

            Scrambler _scrambler;

            std::vector<uint8_t> _buffer;
            int _rotation = -1;
            std::size_t _packets_ok = 0;
            std::size_t _crc_errors = 0;
            std::size_t _corrected_bits = 0;

            ViterbiDecoder _viterbi;
            std::vector<uint8_t> _coded;
            std::vector<uint8_t> _decoded;

            int _match_sync(std::size_t pos) const
            {
                uint32_t word = 0;
                for (std::size_t i = 0; i < SYNC_NIBBLES; ++i)
                    word = (word << 4) | (_buffer[pos + i] & 0x0F);

                // Sync nie je kodovany, preto tolerujeme par chybnych bitov.
                // Rotovane sync slova sa navzajom lisia v 16 bitoch, 4 chyby su bezpecne.
                for (int k = 0; k < 4; ++k)
                    if (std::popcount(word ^ SYNC_ROT[k]) <= SYNC_MAX_BIT_ERRORS)
                        return k;
                return -1;
            }

            uint8_t _read_raw_u8(std::size_t pos, int k)
            {
                const auto &inv = INVERSE[k];
                return static_cast<uint8_t>((inv[_buffer[pos] & 0x0F] << 4) |
                                            inv[_buffer[pos + 1] & 0x0F]);
            }

            // Precita n kodovanych bajtov (2 nibble na bajt) od pozicie pos do _coded
            void _read_coded(std::size_t pos, std::size_t n, int k)
            {
                _coded.resize(n);
                for (std::size_t i = 0; i < n; ++i)
                    _coded[i] = _read_raw_u8(pos + 2 * i, k);
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

    static constexpr float sampling_frequency    = 2'400'000;
    static constexpr float pll_bn_ts_acquisition = 0.06f;
    static constexpr float pll_bn_ts_tracking    = 0.015f;
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::sub);

    sock.set(zmq::sockopt::rcvhwm, 16);
    sock.set(zmq::sockopt::rcvbuf, 128 * 1024);
    sock.set(zmq::sockopt::subscribe, "");

    sock.connect("tcp://127.0.0.1:5555");
    std::cerr << "[START] SUB src=zmq://127.0.0.1:5555  fs=2400000  rs=300000  sps=8" << std::endl;

    Modem::ModemStateMachine state_machine;
    Modem::FrequencyEstimator coarse_estimator(sampling_frequency);
    Modem::FIRFilter low_pass_filter(Modem::lpf_taps.data(), Modem::lpf_taps.size());
    Modem::FIRFilter rrc_filter(Modem::rrc_taps.data(), Modem::rrc_taps.size());
    Modem::DDPLL pll_loop(0.06f, 1.0f); // kD needs callibration
    Modem::GardnerTED timing_loop(0.005, 8);
    Modem::AGC agc(0.8f, 0.4, 1.15e-5f, 1e-4f, 1e4f);
    Modem::DDAGC dd_agc(1, 0.016, 0.5, 2);
    Modem::BitSlicer slicer;
    Modem::FrameSynchronizer frame_synchronizer;
    Modem::ConstellationAnalyzer const_analyzer;

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


    auto apply_state_machine_commands = [&]()
    {
        Modem::ModemStateMachine::event events = state_machine.get_event();
        Modem::ModemStateMachine::task tasks = state_machine.get_task();

        // Events: one-shot, executed once and then cleared
        if (events.reset_frequency_estimator)
        {
            coarse_estimator.reset();
        }

        if (events.reset_loops)
        {
            pll_loop.reset();
            dd_agc.reset();
        }

        if (events.reset_mer)
        {
            const_analyzer.reset();
        }

        state_machine.clear_events();

        // Levels: valid for the whole state, safe to apply on every block
        if (tasks.freeze_agc)
        {
            agc.freeze();
        }
        else
        {
            agc.unfreeze();
        }

        if (tasks.freeze_dd_agc)
        {
            dd_agc.freeze();
        }
        else
        {
            dd_agc.unfreeze();
        }

        if (tasks.freeze_dd_pll)
        {
            pll_loop.freeze();
        }
        else
        {
            pll_loop.unfreeze();
        }

        if (tasks.freeze_gardner)
        {
            timing_loop.freeze();
        }
        else
        {
            timing_loop.unfreeze();
        }

        if (tasks.allow_update_agc_previous_gain)
        {
            agc.enable_storing_gain();
        }
        else
        {
            agc.disable_storing_gain();
        }

        if (tasks.narrow_loop)
        {
            pll_loop.setBnTs(pll_bn_ts_tracking);
        }
        else
        {
            pll_loop.setBnTs(pll_bn_ts_acquisition);
        }
    };


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
        state_machine.update_samples_in_sampling_frequency(n);

        state_machine.update_peak_to_mean(coarse_estimator.peak_to_mean()); 
        if (coarse_estimator.new_estimate() == true)
        {
            state_machine.update_frequency_estimate();
        }
        // Update frequency _peak_to_mean
        state_machine.update();
        apply_state_machine_commands();


        rrc_filter.process(coarse_frequency_data, matched_filtered_data);
        agc.process(matched_filtered_data, agc_data);

        timing_loop.process(agc_data, time_synchronized_data);
        pll_loop.process(time_synchronized_data, phase_synchronized_data);
        dd_agc.process(phase_synchronized_data, dd_agc_data);
        const_analyzer.process(dd_agc_data);

        if (const_analyzer.new_metrics() == true)
        {
            state_machine.update_mer(const_analyzer.mer_db());
        }

        state_machine.update();
        apply_state_machine_commands();


        slicer.process(dd_agc_data, bits);
        frame_synchronizer.process(bits, packets);

        //write_iq_file("symbols.iq", dd_agc_data);
        //write_video(packets, video);
        write_video_to_stdout(packets);
    }
}