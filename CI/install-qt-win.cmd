if exist %QtBaseDir% (
    echo "Qt directory found. Download skipped."
) else (
    echo "Qt directory not found. Downloading..."
    curl -kLO https://tt2468.net/dl/Qt_5.15.2.7z -f --retry 5 -C -
    7z x Qt_5.15.2.7z -o%QtBaseDir%
)
dir %QtBaseDir%
