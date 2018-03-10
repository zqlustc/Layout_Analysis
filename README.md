# Layout_Analysis
Document Layout Analysis Projects
2 algorithms for Layout Analysis preprocess: RLSA X_YCut
Dependencies:OpenCV 3.0+
compile with g++ :g++ -std=c++11 RLSA.cpp -o RLSA `pkg-config --libs --cflags opencv` -ldl
                  or g++ -std=c++11 X_YCut.cpp -o X_YCut `pkg-config --libs --cflags opencv` -ldl
reference: 1.Wong K Y, Casey R G, Wahl F M. Document analysis system[J]. Ibm Journal of Research & Development, 2011, 26(26):647-656.
           2.Ha J, Haralick R M, Phillips I T. Recursive X-Y cut using bounding boxes of connected components[C]// International Conference on Document Analysis and Recognition. IEEE, 1995:952-955 vol.2.
           
身份证识别案例：https://baijiahao.baidu.com/s?id=1593488825686786590&wfr=spider&for=pc

