#! /usr/bin/perl


print "Running test : $ARGV[0] \n";
open IN, "< $ARGV[0]";


my $title = <IN>;
chomp $title;
my $call = <IN>;
chomp $call;

my $header;
my @nominal ;




while (my $line = <IN>) {
    # Note that the final reported Statistic is what will be taken
    if ($line =~ /Model ,\|S\| /) {
	$header = $line;
	$line = <IN> or die "Unexpected end of file after stats readout";
	chomp $line;
	@nominal = split (/\,/,$line);
    }
}

close IN;

# Now run the tool

my $tmpfile = "$ARGV[0].tmp";


# print "syscalling : $call \n";
print "##teamcity[testStarted name='$title']\n";

my @tested;

my @outputs = ();

open IN, "($call) |" or die "An exception was raised when attempting to run "+$call+"\n";
while (my $line = <IN>) {
#  print "$line";
  push (@outputs,$line);
  if ($line =~ /Model ,\|S\| /) {
    $line = <IN> or die "Unexpected end of file after stats readout";
    push (@outputs,$line);
    chomp $line;
    @tested = split (/\,/,$line);
#    last;
  }
}

print "@outputs\n";

if ($call =~ /-reachable/) {
    # so state count may not be accurate if we got interruption. However 0 != positive integer
    if ( (@nominal[1]==0 && @tested[1] > 0 ) || (@nominal[1]>0 && @tested[1]==0 ) ) {
	  print "\n##teamcity[testFailed name='$title' message='reachability test regression detected' details='' expected='@nominal[1]' actual='@tested[1]'] \n";
	  print "Expected :  @nominal[1]  Obtained :  @tested[1] \n";
    } else {
	print "##teamcity[buildStatisticValue key='testDuration' value='@tested[2]']\n";
	print "##teamcity[buildStatisticValue key='testMemory' value='@tested[3]']\n";
	print "Test successful : $title \n";
	print "Control Values/Obtained : \n$title\n@nominal\n@tested\n";
    }
} else {
    if ( @nominal[1] != @tested[1] ) {
	print "\n##teamcity[testFailed name='$title' message='regression detected' details='' expected='@nominal[1]' actual='@tested[1]'] \n";
	print "Expected :  @nominal[1]  Obtained :  @tested[1] \n";
    } else {
	print "##teamcity[buildStatisticValue key='testDuration' value='@tested[2]']\n";
	print "##teamcity[buildStatisticValue key='testMemory' value='@tested[3]']\n";
	print "Test successful : $title \n";
	print "Control Values/Obtained : \n$title\n@nominal\n@tested\n";
    }
}

print "##teamcity[testFinished name='$title']\n";

