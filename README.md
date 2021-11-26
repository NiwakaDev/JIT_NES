# JIT_NES
にわかなJITファミコンエミュレータ

![スクリーンショット 2021-11-26 9 26 59](https://user-images.githubusercontent.com/61189782/143510230-0ba7f3eb-74c0-4d09-b3b4-875486a9ec02.png)


<h2>問題点</h2>
1フレームしか描画しません。Jitプログラムが32bitで動作するため、手元にある64bitGUIライブラリが使えません。そこで、Jitエミュレータ上で1フレーム分描画したら、64bitGUIライブラリにそのフレームを渡すという二段階構成になっています。

<h2>参考</h2>
<ul>
<li>http://essen.osask.jp/?a21_txt02_2</li>
<li>https://jahej.com/alt/2011_06_12_jit-cpu-emulation-a-6502-to-x86-dynamic-recompiler-part-1.html</li>
</ul>
