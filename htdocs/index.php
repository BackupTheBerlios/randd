<html>
<title>RandD</title>
<h1>RandD - Random Daemon</h1>


<table width="100%">
<tr><td width="15%" bgcolor="#c0c0ff" valign="top">

This project is hosted by
<a href="http://developer.berlios.de">
<img src="http://developer.berlios.de/sflogo.php?group_id=0&type=1" width="124" height="32" border="0" alt="BerliOS Logo"></A>

<hr>
<!-----------------------------MENU-------------------->

</td><td width="*" valign="top">
<!-----------------------------CONTENT----------------->

<h2>Purpose</h2>

RandD is a program that collects random bits from various sources to provide them to programs that need them. These range from scientific applications to high-speed
simulators to cryptographic applications. RandD can be configured for different input methods (and combinations of them), speeds and qualities.

<h3>Input methods</h3>

<ul>
<li>Libc Pseudo Random Generator (rand(3) function)
<li>/dev/(u)random
<li>noise at soundcard (sound samples always contain some bits of random)
<li>other RandD's or EGD's (planned)
<li>special random hardware (planned)
</ul>

<h3>Output methods</h3>

<ul>
<li>named pipe
<li>EGD-protocol (used eg. by GnuPG)
</ul>


<h3>Cryptographic Applications</h3>

High traffic HTTPS servers or servers for other encrypted protocols can make use of high speed random sources.<p>

<b>Help needed:</b> I need some descriptions how to configure various servers for use of named pipes or EGD protocol. If you know other protocols used in common crypto
systems, please tell me.<p>

<h3>Scientific Applications</h3>

Most applications are absolutely fine with the "predictable" rand-function and few random bits from the system clock at startup. Other properties of the random bit stream
are more important (ideal distribution, also known as "white noise"). A hardware random source will always deliver non-ideally distributed but real random (pink noise).
For the few applications that need a higher unpredictability of the random (eg. really complicated Monte-Carlo simulations) should be fine with seeding rand from the
random server after few thousand samples. If highest quality is needed it might be a good idea to combine a hardware source and the rand function by xor'ing them (RandD
is already able to do that) - thus a ideally distributed true random source can be created very easily.


<h2>Project Status</h2>

The project is heading for the first Alpha release.



</td></tr>
</table>


</html>
