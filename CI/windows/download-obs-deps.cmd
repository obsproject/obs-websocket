if exist %DEPS_BASE_PATH% (
    echo "OBS dependencies found. Download skipped."
) else (
    echo "OBS dependencies not found. Downloading..."
    curl -o %DEPS_BASE_PATH%.zip -kLO https://cdn-fastly.obsproject.com/downloads/dependencies2019.zip -f --retry 5 -C -
    7z x %DEPS_BASE_PATH%.zip -o%DEPS_BASE_PATH%
)
