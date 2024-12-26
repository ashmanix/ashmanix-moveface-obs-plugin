#!/usr/bin/env zsh

BUILD_FILE_LOCATION=./build_macos/RelWithDebInfo/ashmanix-moveface-obs-plugin.plugin
BUILD_DESTINATION_FOLDER="$HOME/Library/Application Support/obs-studio/plugins/"
APP_NAME="OBS"

RED="\033[0;31m"
GREEN="\033[0:32m"
NC="\033[0m" # No Color
CLEAR_LINE="\r\033[K"  # Carriage return with clear line


# Parse flags using getopts
while getopts "d:vh" flag; do
    case "$flag" in
        b) BUILD_FILE_LOCATION="$OPTARG" ;;  # -b sets the build path
        d) BUILD_DESTINATION_FOLDER="$OPTARG" ;;  # -d sets the build path
        h)
            echo "Usage: $0 [-b build path] [-d destination] [-h]"
            echo "  -b build path     Specify the build path"
            echo "  -d destination    Specify the destination path"
            echo "  -h                Display this help message"
            exit 0
            ;;
        *) 
            echo "Invalid option. Use -h for help."
            exit 1
            ;;
    esac
done


BUILD_PRESET="macos"
echo "Using preset: $BUILD_PRESET"



echo "Starting build and run script..."


# Run the build macos script
echo -ne "${CLEAR_LINE}• Running cmake compile script..."

# Temporary file to store output
temp_build_output=$(mktemp)

cmake --preset $BUILD_PRESET >$temp_build_output 2>&1

if [ $? -ne 0 ]; then
    echo "${CLEAR_LINE}${RED}X${NC} CMake compile failed! Ending script. Output:"
    cat $temp_build_output
    exit 1
else
    echo "${CLEAR_LINE}${GREEN}✓${NC} CMake compile completed successfully!"
fi

# Run the cmake build script
echo -ne "• Running cmake compile script..."
cmake --build --preset $BUILD_PRESET --target clean >"$temp_build_output" 2>&1
cmake --build --preset $BUILD_PRESET >"$temp_build_output" 2>&1

if [ $? -ne 0 ]; then
    echo "${CLEAR_LINE}${RED}X${NC} CMake build failed! Ending script. Output:"
    cat "$temp_build_output"
    exit 1
else
    echo "${CLEAR_LINE}${GREEN}✓${NC} CMake build completed successfully!"
fi


# Copy the plugin to the correct destination folder
echo -ne "• Copying build folder to application folder${CLEAR_LINE}"
cp -r "$BUILD_FILE_LOCATION" "$BUILD_DESTINATION_FOLDER"
if [ $? -ne 0 ]; then
    echo "${CLEAR_LINE}${RED}X${NC} Copying plugin to application folder failed! Ending script."
    exit 1
else
    echo "${CLEAR_LINE}${GREEN}✓${NC} Plugin copied to application folder successfully"
fi


# If OBS is open then close it
if pgrep $APP_NAME >/dev/null; then
    echo -ne "• $APP_NAME is currently open. Now closing...."
    # Temporary file to store output
    temp_close_output=$(mktemp)
    SCRIPT="
        try
            tell application \"$APP_NAME\" to quit
            on error errorMessage number errorNumber 
            if (errorNumber is -128) or (errorNumber is -1711) then 
            set tekst to "Stop" 
            end if 
        end try
    "
    osascript -e $SCRIPT >>"$temp_close_output" 2>&1

    if [ $? -ne 0 ]; then
        echo "${CLEAR_LINE}${RED}X${NC} Error closing $APP_NAME app! Output:"
        cat "$temp_close_output"
        exit 1
    fi

    echo "${CLEAR_LINE}${GREEN}✓${NC} $APP_NAME closed successfully!"
    sleep 1
fi


# Open OBS
echo -ne "• Opening $APP_NAME app"
open -a $APP_NAME
if [ $? -ne 0 ]; then
    echo "${CLEAR_LINE}${RED}X${NC} Error opening $APP_NAME app! Ending script."
    exit 1
else
    echo "${CLEAR_LINE}${GREEN}✓${NC} $APP_NAME opened successfully!"
fi

echo "${GREEN}✓${NC} End of build and run script!"
