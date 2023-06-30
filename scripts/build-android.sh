cd ../android
./gradlew app:clean
./gradlew sdk:assembleRelease
./gradlew app:assembleRelease
