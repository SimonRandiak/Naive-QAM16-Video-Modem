# Original code from https://github.com/arif-du/Digital-Signal-Processing-with-Python/blob/main/16_FIR_filter.ipynb

import numpy as np
from scipy.signal import firwin, freqz
import matplotlib.pyplot as plt

# Filter requirements
fs = 2.4e6 # Sample rate, Hz
cutoff = 315_000 # Desired cutoff frequency, Hz
numtaps = 101 # Size of the FIR filter

# Compute the filter coefficients using the firwin function
taps = firwin(numtaps, cutoff, window='hamming', pass_zero=True, fs=fs)

# Compute the frequency response of the filter
w, h = freqz(taps, 1, worN=8192, fs=fs)
h_db = 20*np.log10(np.abs(h) + 1e-12)

print(f"potlacenie od 400 kHz: {-h_db[w >= 400e3].max():.1f} dB")

def to_cpp_array(taps, name="lpf_taps", ctype="float", per_line=4):
    n = len(taps)
    suffix = "f" if ctype == "float" else ""
    lines = ["#pragma once",
             "#include <array>",
             "",
             f"constexpr std::array<{ctype}, {n}> {name} = {{"]
    for i in range(0, n, per_line):
        chunk = taps[i:i + per_line]
        lines.append("    " + ", ".join(f"{v: .9e}{suffix}" for v in chunk) + ",")
    lines.append("};")
    return "\n".join(lines)

print(to_cpp_array(taps))