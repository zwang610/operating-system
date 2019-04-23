touch f.txt
echo 0 > f0.txt
echo 1 > f1.txt
echo 2 > f2.txt
touch out.txt
touch err.txt

echo "--TEST 1"
./simpsh --verbose --rdonly f0.txt --wronly f1.txt --wronly f2.txt --command 0 1 2 cat >out.txt 2>err.txt
diff -q f.txt out.txt
if [ $? -ne 0 ] 
then 
    echo "test1 passed" 
else 
    echo "test1 failed"
fi

rm f.txt f0.txt f1.txt f2.txt out.txt err.txt

touch f.txt
echo 0 > f0.txt
echo 1 > f1.txt
echo 2 > f2.txt
touch out.txt
touch err.txt

echo "--TEST 2"
./simpsh --rdonly f0.txt --wronly f1.txt --wronly f2.txt --command 0 1 2 cat --verbose >out.txt 2>err.txt
diff -q f.txt out.txt
if [ $? -eq 0 ] 
then 
    echo "test2 passed" 
else 
    echo "test2 failed"
fi

rm f.txt f0.txt f1.txt f2.txt out.txt err.txt

touch f.txt
echo 0 > f0.txt
echo 1 > f1.txt
echo 2 > f2.txt
touch out.txt
touch err.txt

echo "--TEST 3"
./simpsh --rdonly f0.txt --wronly f1.txt --wronly f2.txt --command c d 2 cat >out.txt 2>err.txt
if [ -s err.txt ] 
then 
    echo "test3 passed" 
else 
    echo "test3 failed"
fi

rm f.txt f0.txt f1.txt f2.txt out.txt err.txt
echo "DONE ..."
cd ..

