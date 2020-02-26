if not exist %DepsBasePath%.zip curl -kLO https://obsproject.com/downloads/dependencies2017.zip -f --retry 5 -C -
7z x %DepsBasePath%.zip -o%DepsBasePath%