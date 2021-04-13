if exist %QT_BASE_DIR% (
    echo "Qt directory found. Download skipped."
) else (
    echo "Qt directory not found. Downloading..."
    curl -kLO https://tt2468.net/dl/Qt_5.15.2.7z -f --retry 5 -C -
    7z x Qt_5.15.2.7z -o%QT_BASE_DIR%
)
dir %QT_BASE_DIR%
