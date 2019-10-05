#!/bin/sh

echo "Åúï∂éöóÒÇÃêÿÇËèoÇµ"
#==========================================================================================================*
sql_template="select aaa,bbb,ccc from T01 where ddd = '@2@' h = '@4@' and fff=@10@ order by @100@,@102@"
#==========================================================================================================*
plcfdr=( $( echo ${sql_template} | grep -o '@[1-9]@' ) \
         $( echo ${sql_template} | grep -o '@[1-9][0-9]@' ) \
         $( echo ${sql_template} | grep -o '@[1-9][0-9][0-9]@' ) )
for((idx=0; idx<${#plcfdr[@]}; idx++))
do
    printf "\${plcfdr[%d]}->%-5s  [%d]\n" $idx ${plcfdr[idx]} ${plcfdr[idx]//@/}
done
echo


echo "ÅúSQLï∂ÇÃïœä∑"
#==========================================================================================================*
sql_template="select aaa,bbb,ccc from T01 where ddd = '@2@' h = '@4@' and fff=@1@ order by @3@,@4@"
#==========================================================================================================*
plcfdr=( $( echo ${sql_template} | grep -o '@[1-9]@' ) \
         $( echo ${sql_template} | grep -o '@[1-9][0-9]@' ) \
         $( echo ${sql_template} | grep -o '@[1-9][0-9][0-9]@' ) )

while read line
do
    values=( ${line//,/ } )

    # DISP(Unnecessary)
    echo "---------"
    for((i=0; i<${#values[@]}; i++))
    do
        echo ${values[i]}
    done

    # EDIT
    sql_work=${sql_template}
    for((i=0; i<${#plcfdr[@]}; i++))
    do
        x=$(( ${plcfdr[i]//@/} - 1 ))
        sql=${sql_work//${plcfdr[i]}/${values[x]}}
        sql_work=$sql
    done
    echo $sql
    echo

done <<< cat <<EOF
12,23,aaaa,BB
21,6,bb,cc
9,4,cc,aa
EOF

