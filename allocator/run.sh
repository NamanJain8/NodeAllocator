#!/bin/bash
array="${@}"
proc_count=$1
ppn=$2
binary=${array[@]:4}
echo $binary

touch metadata.log
touch minimd.log
echo "" > minimd.log

echo "" >> metadata.log
echo "" >> metadata.log
echo "======================================================================" >> metadata.log
echo "proc count : $proc_count BINARY : $binary" >> metadata.log
echo "======================================================================" >> metadata.log
echo "======================================================================" >> metadata.log

~/UGP/allocator/src/allocator.out $proc_count $ppn >> metadata.log
echo ""
echo "Selected hosts are"
cat hosts


# echo "======================================================================"
start=`date +%s%3N`
# echo "MPI PROGRAM STARTS : $start "
# echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
if (( $ppn == 0 ))
then
   mpiexec -n $proc_count -hostfile hosts $binary >> minimd.log
else
    mpiexec -n $proc_count -ppn $ppn -hostfile hosts $binary >> minimd.log
fi
end=`date +%s%3N`
runtime=$((end-start))
# echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "MPI PROGRAM :: Total Time : $runtime"
# echo "======================================================================"

echo ""
echo "Selected hosts are"
cat comphosts
# echo "======================================================================"
start=`date +%s%3N`
# echo "MPI PROGRAM STARTS WITT MAX COMPUTES: $start "
# echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
if (( $ppn == 0 ))
then
   mpiexec -n $proc_count -hostfile comphosts $binary >> minimd.log
else
    mpiexec -n $proc_count -ppn $ppn -hostfile comphosts $binary >> minimd.log
fi
end=`date +%s%3N`
runtime=$((end-start))
# echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "MPI PROGRAM MAX COMPUTE :: Total Time : $runtime"
# echo "======================================================================"



numhosts=`wc -l < hosts`
cat ~/.eagle/livehosts.txt | sort -R | head -n $numhosts > randomhosts
echo ""
echo "Randomly Selected hosts are"
cat randomhosts
# echo "======================================================================"
start=`date +%s%3N`
# echo "MPI PROGRAM STARTS ON RANDOM : $start "
# echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
if (( $ppn == 0 ))
then
   mpiexec -n $proc_count -hostfile randomhosts $binary >> minimd.log
else
    mpiexec -n $proc_count -ppn $ppn -hostfile randomhosts $binary >> minimd.log
fi
end=`date +%s%3N`
runtime=$((end-start))
# echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "MPI PROGRAM RANDOM :: Total Time : $runtime"
# echo "======================================================================"

numlivehosts=`wc -l < /users/btech/akashish/.eagle/livehosts.txt`	
selectedend=`shuf -i $numhosts-$numlivehosts -n 1`	
cat /users/btech/akashish/.eagle/livehosts.txt | head -n $selectedend | tail -n $numhosts > sequencehosts	
echo ""	

echo "Random sequence Selected hosts are"
cat sequencehosts
# echo "======================================================================"	
start=`date +%s%3N`	
# echo "MPI PROGRAM STARTS ON SEQUENCE : $start "	
# echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"	
if (( $ppn == 0 ))	
then	
   mpiexec -n $proc_count -hostfile sequencehosts $binary >> minimd.log	
else	
    mpiexec -n $proc_count -ppn $ppn -hostfile sequencehosts $binary  >> minimd.log
fi	
end=`date +%s%3N`
runtime=$((end-start))
# echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"	
echo "MPI PROGRAM Sequence  :: Total Time : $runtime"
# echo "======================================================================"