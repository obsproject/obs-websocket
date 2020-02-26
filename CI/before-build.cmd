cd ./build32
cmake -G "Visual Studio 14 2017" -DQTDIR="%QTDIR32%" -DLibObs_DIR="C:\projects\obs-studio\build32\libobs" -DLIBOBS_INCLUDE_DIR="C:\projects\obs-studio\libobs" -DLIBOBS_LIB="C:\projects\obs-studio\build32\libobs\%build_config%\obs.lib" -DOBS_FRONTEND_LIB="C:\projects\obs-studio\build32\UI\obs-frontend-api\%build_config%\obs-frontend-api.lib" ..
cd ../build64
cmake -G "Visual Studio 14 2017 Win64" -DQTDIR="%QTDIR64%" -DLibObs_DIR="C:\projects\obs-studio\build64\libobs" -DLIBOBS_INCLUDE_DIR="C:\projects\obs-studio\libobs" -DLIBOBS_LIB="C:\projects\obs-studio\build64\libobs\%build_config%\obs.lib" -DOBS_FRONTEND_LIB="C:\projects\obs-studio\build64\UI\obs-frontend-api\%build_config%\obs-frontend-api.lib" ..
