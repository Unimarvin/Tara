#! /bin/bash

# copy files to builddir
$MKDIR_P $builddir/soundness
cp $testdir/soundness/* $builddir/soundness

echo ""

echo "UML2oWFN compiler for soundness analysis tests"
echo "[strong termination and safeness, LoLA, taskfiles]"
echo "uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -a safe -p taskfile -f lola -o"
uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -a safe -p taskfile -f lola -o
result1=$?

if [ ! -f $builddir/soundness/check_TestSuite_soundness.sh ] ; then
  result1=1
  echo "[fail] no shell script generated"
fi
if [ ! -f $builddir/soundness/TestSuite_soundness.BOMcncpt1__ProcessPinDisjoint_sound_ifStop.lola.fin.task -o ! -f $builddir/soundness/TestSuite_soundness.BOMcncpt1__ProcessPinDisjoint_sound_ifStop.lola.safe.task ] ; then
  result2=1
  echo "[fail] task files not generated"
fi
rm -f $builddir/soundness/*.task
rm -f $builddir/soundness/*.lola
rm -f $builddir/soundness/*.sh

echo ""
echo "[strong termination, LoLA, no taskfiles]"
echo "uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -f lola -o"
uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -f lola -o &> /dev/null
result2=$?
if [ ! -f $builddir/soundness/check_TestSuite_soundness.sh ] ; then
  result2=1
  echo "[fail] no shell script generated"
fi
if [ -f $builddir/soundness/TestSuite_soundness.BOMcncpt1__ProcessPinDisjoint_sound_ifStop.lola.fin.task -o -f $builddir/soundness/TestSuite_soundness.BOMcncpt1__ProcessPinDisjoint_sound_ifStop.lola.safe.task ] ; then
  result2=1
  echo "[fail] task files generated"
fi
rm -f $builddir/soundness/*.lola
rm -f $builddir/soundness/*.sh

echo ""
echo "[strong termination and safeness, LoLA, taskfiles, orJoin termination semantics]"
echo "uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -a safe -a orJoin -p taskfile -f lola -o"
uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -a safe -a orJoin -p taskfile -f lola -o &> /dev/null
result3=$?
if [ ! -f $builddir/soundness/check_TestSuite_soundness.sh ] ; then
  result3=1
  echo "[fail] no shell script generated"
fi
rm -f $builddir/soundness/*.task
rm -f $builddir/soundness/*.lola
rm -f $builddir/soundness/*.sh

echo ""
echo "[strong termination and safeness, LoLA, taskfiles, weak data-flow termination semantics]"
echo "uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -a safe -a removePinsets -p taskfile -f lola -o"
uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -a safe -a removePinsets -p taskfile -f lola -o &> /dev/null
result4=$?
if [ ! -f $builddir/soundness/check_TestSuite_soundness.sh ] ; then
  result4=1
  echo "[fail] no shell script generated"
fi
rm -f $builddir/soundness/*.task
rm -f $builddir/soundness/*.lola
rm -f $builddir/soundness/*.sh

echo ""
echo "[Woflan (TPN)]"
echo "uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -a wfNet -f tpn -o"
uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -a wfNet -f tpn -o &> /dev/null
result5=$?
rm -f $builddir/soundness/*.tpn

echo ""
echo "[CTL model checking]"
echo "uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -p ctl -f lola -o"
uml2owfn -i $builddir/soundness/TestSuite_soundness.xml -p filter -a soundness -p ctl -f lola -o &> /dev/null
result6=$?

must_filecount_filter=12
agef_count=$(ls $builddir/soundness/*.lola | xargs grep "ALLPATH ALWAYS ( EXPATH EVENTUALLY (" | wc -l)
if [ ! $agef_count -eq $must_filecount_filter ] ; then
  result6=1
  echo "[fail] CTL properties not generated for all processes"
fi
lola_mc_count=$(grep "lola-mc" $builddir/soundness/check_TestSuite_soundness.sh | wc -l)
if [ ! $lola_mc_count -eq $must_filecount_filter ] ; then
  result6=1
  echo "[fail] not calling CTL model checker for CTL analysis"
fi
rm -f $builddir/compiler/*.lola
rm -f $builddir/compiler/*.task
rm -f $builddir/compiler/*.sh


# remove files from builddir
rm -f $builddir/soundness/TestSuite_soundness.txt
rm -f $builddir/soundness/TestSuite_soundness.xml


if test \( $result1 -eq 0 -a $result2 -eq 0 -a $result3 -eq 0 -a $result4 -eq 0 -a $result5 -eq 0 -a $result6 -eq 0 \)
then
  result=0
else
  result=1
fi

exit $result
