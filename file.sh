#!/bin/bash

FILES=`zenity --file-selection --multiple --separator="," --title="se"`

case $? in
         0)
                echo "\"$FILES\" is selected"
		
#		if [ ${#FILES[@]} -gt 0 ]; then
#			for FILE in "${FILES[@]}"; do
#				echo "ls -l sam"
#				ls -l "$FILE"
#			done
#		fi
#		;;
		if [ -n "$FILES" ]; then
			echo "read"
			#echo "$FILES" | sed 's/\\n/\n/'
			#readarray -t selected_files <<< "$FILES" | sed 's/\\n/\n/'
			IFS=',' read -r -a selected_files <<< "$FILES"
			#printf "%s\n" "${selected_files[0]}"
			for file in "${selected_files[@]}"; do
				#echo "ls -l sita"
				#echo ls -l "$file"
				#printf "%s\n" "${selected_files[@]}"
				ls -l "$file"
				#../../root/root_v6.34.00-rc1.Linux-ubuntu22.04-x86_64-gcc11.4/root/bin/root.exe Plot_PolyFIT_250203.C\(\"$file\",100,60000\)
				#gnome-terminal --tab --title="run$file" -- bash -ic '../../root/root_v6.34.00-rc1.Linux-ubuntu22.04-x86_64-gcc11.4/root/bin/root.exe Plot_PolyFIT_250203.C\(\"$file\",100,60000\); exec bash'
				export CURRENT_FILE="$file"
				# gnome-terminal内で環境変数にアクセス				
				gnome-terminal --tab --title="run$file" -- bash -ic 'export CURRENT_FILE="$CURRENT_FILE";root -b -q Plot_PolyFIT_250401.C\(\"$CURRENT_FILE\",100,60000\);exec bash'

				echo "-----"
			done
		echo "exec"
		fi
		;;
		
		#root Plot_PolyFIT_250203.C\(\"ROOT/Th0100/[Run#].root\",Ql_MIN,Ql_MAX\)
         1)
                echo "ファイルが選択されませんでした。";;
        -1)
                echo "予期せぬエラーが発生しました。";;
esac
