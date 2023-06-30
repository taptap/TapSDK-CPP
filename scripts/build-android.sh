cd ../android || exit
./gradlew app:clean
./gradlew sdk:assembleRelease
./gradlew app:assembleRelease
