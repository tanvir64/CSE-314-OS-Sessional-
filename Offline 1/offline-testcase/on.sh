#!/bin/bash
total_arguments=$#
current_directory=""
filename=""
start_directory=$PWD
IFS=$'\n'
echo "Number of arguments: $total_arguments"
if (($total_arguments == 2));then
    echo "Both the directory and the filename has been provided"
    if [ -d "$1" -a -f "$2" ];then
        current_directory=$1
        filename=$2
    fi
elif (($total_arguments == 1));then
    if [ -d "$1" ];then
        current_directory="$1"
        echo "Please specify the filename containing the file types you want to ignore"
        until [ "$filename" != "" ]
        do
            echo -n "Please provide a valid input file name"
            read name
            if [ $name = "*.txt"];then
                filename="$name"
            fi
        done
    elif [ -e "$1" ];then
        current_directory=$PWD
        filename=$1
    fi    
else
    echo "No command line arguments given.Please specify the working directory name(optional) and input file name!!!"
fi
declare -a ignored_type
n=0
while read line;do 
    ignored_type[$n]=$line
    n=$((n+1))
done < "$filename"

if [ "$current_directory"!="$PWD" ];then
    cd "$current_directory"
fi
# getting the list of all file extensions
IFS=$'\n'
allFilesExt=( $(find "$current_directory" -type f | sed -rn 's|.*/[^/]+\.([^/.]+)$|\1|p' |sort -u |uniq))
m=0
declare -a extensions
# printf "${allFilesExt[*]}"
for elem in "${allFilesExt[@]}";do  
    found=0
    for item in "${ignored_type[@]}";do
        if [ "$item" = "$elem" ];then 
            found=$((found+1))
        fi
    done                
    if (($found==0));then
        extensions[$m]=$elem
        m=$((m+1))
    fi    
done
printf "${allFilesExt[*]}"

out="../1705064_out_dir"
rm -f -r $out
mkdir $out
csv_dir="../1705064_output.csv"
rm -f $csv_dir

for i in "${extensions[@]}"
do
    directory="$out/$i"
    mkdir -p $directory && touch $directory/desc_$i.txt
done
for type in "${extensions[@]}";do
    find "$current_directory" -name "*.$type" -exec cp {} $out/$type/ \;
done

IFS=$'\n'
extensionless_file=($(find "$current_directory" -type f ! -name "*.*"))
others_count=${#extensionless_file[*]}
# printf "%s\n" "${extensionless_file[@]}"
# echo "Others: $others_count"

if (($others_count > 0));then
    mkdir "$out/others" && touch "$out/others/desc_others.txt"
    extensions+=("others")
fi

for item in "${extensionless_file[@]}";do
    cp "$item" "$out/others/"
done

######writing to csv file######

echo "file_type, no_of_files" >> "$csv_dir"
for file_type in "${extensions[@]}";do    
    if [ "$file_type" = "others" ];then    
        echo "others,$others_count" >> "$csv_dir"
    else         
        count_file=($(find "$out/$file_type" -type f -name "*.$file_type" -printf x | wc -c))
        if [ "$file_type" = "txt" ];then        
            echo "$file_type,$(($count_file-1))" >> "$csv_dir"
        else 
            echo "$file_type,$count_file" >> "$csv_dir"
        fi
    fi
done      
ignore_count=0
for file_type in "${ignored_type[@]}";do
    count_file=($(find "$current_directory" -mindepth 1 -type f -name "*.$file_type" -printf x | wc -c))
    ignore_count=`expr $ignore_count + $count_file`
done        
echo "ignored,$ignore_count" >> "$csv_dir"
#####Writing the relative path of the file#####
working_directory=`pwd | awk -F '/' '{print $NF}'`
# echo "$working_directory"
for i in "${extensions[@]}";do
    files=()
    dup=0
    if [ $i != "others" ];then    
        abs_path=($(find "$current_directory" -type f -name "*.$i"))
        for((j=0;j<${#abs_path[*]};j++));do
            file=$(basename "${abs_path[$j]}" ".$i")
            # echo $file            
            files+=("$file")
            for((k=0;k<$((${#files[*]}-1));k++));do
                # echo "$files[$k],then $file"
                if [[ "${files[$k]}" = "$file" ]];then
                    # echo "${files[$k]} = $file"
                    dup=1
                fi
            done            
            if (($dup == 1));then
                echo "Duplicate found"
            else
                var=($(realpath --relative-to="$start_directory" "${abs_path[$j]}"))
                echo "$var" >> "$out/$i/desc_$i.txt"
            fi
        done
    else
        abs_path=($(find "$current_directory" -type f ! -name "*.*"))
        for((j=0;j<${#abs_path[*]};j++));do
            var=($(realpath --relative-to="$start_directory" "${abs_path[$j]}"))
            echo "$var" >> "$out/others/desc_others.txt"
        done
    fi
done