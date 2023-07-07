cd ../android || exit
rm -rf ./sdk/.cxx
./gradlew app:clean
./gradlew sdk:assembleRelease
./gradlew app:assembleRelease
