so to recap:
- `-Ofast` implies `-O3` and `-ffast-math`

- ground truth(?) (Vipre, steam 2.0.5 beta #7, release mode) -> 267.183
- steam 2.0.5 beta #7 (release mode) -> 267.183
- debug build -> 263.671
- server (which was a debug build at the time) -> 263.671
- release build (`-Ofast -ffast-math`) -> 267.183
- release build (`-Ofast` [implied `-ffast-math`]) -> 267.183
- release build (`-O2`) -> 263.671
- release build (`-O2 -ffast-math`) -> 267.183
- release build (`-O3`) -> 263.671
- release build (`-O3`, all fast math flags) -> 267.183
- release build (`-O3`, all fast math flags except `-fexcess-precision=fast`) -> 267.183
- release build (`-O3`, all fast math flags except `-fcx-limited-range`) -> 267.183
- release build (`-O3`, all fast math flags except `-fno-signaling-nans`) -> 267.183
- release build (`-O3`, all fast math flags except `-fno-rounding-math`) -> 267.183
- release build (`-O3`, all fast math flags except `-ffinite-math-only`) -> 267.183
- release build (`-O3`, all fast math flags except `-fno-math-errno`) -> 267.183
- release build (`-O3`, all fast math flags except `-funsafe-math-optimizations`) -> 263.671
- release build (`-O3`, all fast math flags and `-fno-unsafe-math-optimizations`) -> 263.671

breaking down `-funsafe-math-optimizations`:
- with all four flags -> 267.183
- without `-freciprocal-math` -> 263.671
- without `-fassociative-math` -> 117.017
- without `-fno-trapping-math` -> 117.017
- without `-fno-signed-zeros` -> 117.017
