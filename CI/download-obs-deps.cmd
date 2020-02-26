if not exist %DepsBasePath%.zip curl -o %DepsBasePath%.zip -kLO https://obsproject.com/downloads/dependencies2017.zip -f --retry 5 -C -
7z x %DepsBasePath%.zip -o%DepsBasePath%