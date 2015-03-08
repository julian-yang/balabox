#!/bin/bash

function fileExists {

	if [ ! -f $1 ]; then
    	echo "File $1 not found!"
    	exit 1
	fi
}

function exitUpload {
	if [ ! -d  ./responses ]; then
    		`sudo rm -r "./responses"`
			echo -e "Deleted responses directory\n"
	fi
	exit 1
}

function checkStatus { 

	if echo "$1" | grep "<html>"; then 
		status_code=$(echo "$1" | grep -o -w -E '[[:digit:]]{3}')

		#echo -e "HTTP status: $status_code"
		if [[ "$status_code" = "400" ]]
		then
			echo "HTTP Status Code: 400. Check Input."
			exit 1
		elif [[ "$status_code" = "404" ]]
		then
			echo "HTTP Status Code: 404. Not Found."
			exit 1
		elif [[ "$status_code" = "500" ]]
		then
			echo "HTTP Status Code: 500. Internal Server Error."
			exit 1
		fi
	fi
}