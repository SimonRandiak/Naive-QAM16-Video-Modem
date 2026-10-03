#!/usr/bin/env python3
"""
Zobrazenie IQ dat (interleaved float32 / complex64) ako konstelacny diagram.

Pouzitie:
    python plot_constellation.py symbols.cf32
    python plot_constellation.py rx.cf32 --sps 8 --offset 3 --skip 1000
    python plot_constellation.py rx.cf32 --density

--sps / --offset : ak subor obsahuje vzorky (nie symboly), vyberie sa
                   kazda sps-ta vzorka od zadaneho offsetu
--skip           : preskoci prvych N vzoriek (napr. kym sa ustali AGC/sync)
--max            : maximalny pocet zobrazenych bodov
--no-norm        : nenormalizovat na jednotkovy priemerny vykon
--density        : 2D histogram namiesto scatter (lepsie pri vela bodoch)
"""

import argparse
import numpy as np
import matplotlib.pyplot as plt


def qam16_ideal():
    lv = np.array([-3, -1, 1, 3], dtype=np.float32)
    pts = (lv[None, :] + 1j * lv[:, None]).ravel()
    return pts / np.sqrt(10.0)  # priemerny vykon = 1


def main():
    p = argparse.ArgumentParser()
    p.add_argument("file")
    p.add_argument("--sps", type=int, default=1)
    p.add_argument("--offset", type=int, default=0)
    p.add_argument("--skip", type=int, default=0)
    p.add_argument("--max", type=int, default=20000)
    p.add_argument("--no-norm", action="store_true")
    p.add_argument("--density", action="store_true")
    a = p.parse_args()

    iq = np.fromfile(a.file, dtype=np.complex64)
    print(f"Nacitanych {iq.size} vzoriek")

    iq = iq[a.skip:]
    iq = iq[a.offset::a.sps]
    iq = iq[-a.max:]  # posledne body, kde uz je slucka zvycajne zamknuta

    if iq.size == 0:
        raise SystemExit("Ziadne data na zobrazenie.")

    if not a.no_norm:
        iq = iq / np.sqrt(np.mean(np.abs(iq) ** 2))

    ideal = qam16_ideal()

    # Jednoduchy odhad EVM: vzdialenost k najblizsiemu idealnemu bodu
    d = np.abs(iq[:, None] - ideal[None, :])
    err = d.min(axis=1)
    evm = np.sqrt(np.mean(err ** 2))  # ideal ma P = 1
    print(f"EVM ~ {100 * evm:.2f} %  ({20 * np.log10(evm):.1f} dB)")

    fig, ax = plt.subplots(figsize=(7, 7))
    lim = 1.6 * np.max(np.abs(ideal))

    if a.density:
        ax.hist2d(iq.real, iq.imag, bins=300,
                  range=[[-lim, lim], [-lim, lim]], cmap="inferno", cmin=1)
    else:
        ax.scatter(iq.real, iq.imag, s=2, alpha=0.4, label="prijate")

    ax.scatter(ideal.real, ideal.imag, s=80, marker="x", c="red",
               linewidths=2, label="ideal QAM16")

    ax.axhline(0, color="gray", lw=0.5)
    ax.axvline(0, color="gray", lw=0.5)
    ax.set_xlim(-lim, lim)
    ax.set_ylim(-lim, lim)
    ax.set_aspect("equal")
    ax.set_xlabel("I")
    ax.set_ylabel("Q")
    ax.set_title(f"{a.file} — {iq.size} bodov, EVM {100 * evm:.1f} %")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right")
    plt.tight_layout()
    plt.show()
    fig2, ax2 = plt.subplots(figsize=(10, 3))
    ax2.plot(err, lw=0.5)
    ax2.set_xlabel("symbol"); ax2.set_ylabel("|chyba|")
    plt.show()

if __name__ == "__main__":
    main()