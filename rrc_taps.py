import numpy as np

def rcosdesign_sqrt(beta, span, sps):
    delay = span * sps / 2
    t = np.arange(-delay, delay + 1) / sps
    h = np.zeros_like(t)
    eps = np.sqrt(np.finfo(float).eps)

    i0 = np.isclose(t, 0)
    h[i0] = -1 / (np.pi * sps) * (np.pi * (beta - 1) - 4 * beta)

    i1 = np.abs(np.abs(4 * beta * t) - 1) < eps
    h[i1] = 1 / (2 * np.pi * sps) * (
        np.pi * (beta + 1) * np.sin(np.pi * (beta + 1) / (4 * beta))
        - 4 * beta * np.sin(np.pi * (beta - 1) / (4 * beta))
        + np.pi * (beta - 1) * np.cos(np.pi * (beta - 1) / (4 * beta)))

    k = ~(i0 | i1)
    tk = t[k]
    h[k] = -4 * beta / sps * (np.cos((1 + beta) * np.pi * tk)
            + np.sin((1 - beta) * np.pi * tk) / (4 * beta * tk)) \
           / (np.pi * ((4 * beta * tk) ** 2 - 1))

    return h / np.sqrt(np.sum(h ** 2))

h = rcosdesign_sqrt(0.4, 11, 8)

def to_c_array(taps, name="rrc_taps", ctype="float", per_line=4):
    lines = [
             f"static const {ctype} {name}[] = {{"]
    for i in range(0, len(taps), per_line):
        chunk = taps[i:i + per_line]
        if ctype == "float":
            vals = ", ".join(f"{v: .9e}f" for v in chunk)
        else:
            vals = ", ".join(f"{int(v):6d}" for v in chunk)
        lines.append(f"    {vals},")
    lines.append("};")
    return "\n".join(lines)

print(to_c_array(h))