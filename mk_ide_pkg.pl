#!/usr/bin/perl -w
#use strict;
use Getopt::Long;
use File::Find;
use Cwd 'cwd';
use Cwd 'abs_path';

sub build_base_pkg
{
	(my $outpath)=@_;
	my $base_pkg_list="middleware/third_party/arduino/hardware/arduino/mt7697/";

	die("\"$base_pkg_list\" doesn't exist.") if (not -e $base_pkg_list);

	system("rm -rf $outpath/mt7697") if (-e "$outpath/mt7697");

	system("cp -r $base_pkg_list $outpath");

	system("mv $outpath/mt7697/variants/linkit_7697/startup_mt7687.s $outpath/mt7697/variants/linkit_7697/startup_mt7687.S");
}

sub build_system_src
{
	(my $outpath)=@_;

	my @source_list=(
		"driver/chip/mt7687/inc/",
		"driver/chip/inc/",
		"driver/CMSIS/Device/MTK/mt7687/Include/",
		"driver/CMSIS/Include/",
		"kernel/service/inc",
		"kernel/rtos/FreeRTOS/Source/include/",
		"kernel/rtos/FreeRTOS/Source/portable/GCC/ARM_CM4F/",
		"middleware/third_party/lwip/src/include/",
		"middleware/third_party/lwip/ports/include/",
		"driver/board/mt76x7_hdk/wifi/inc/");
    
	my @remove_list=("driver/chip/mt7687/inc/wifi.h");

	$outpath = "$outpath/mt7697/system/linkit_7697/src";

	system("mkdir -p $outpath");
	foreach $item (@source_list) {
		system("mkdir -p $outpath/$item");
		system("rm -rf $outpath/$item") if (-e "$outpath/$item");

		system("cp -r $item $outpath/$item");
	}

	foreach $item (@remove_list) {
		system("rm -rf $outpath/$item") if (-e "$outpath/$item");
	}
}

sub process_obj {
	#print abs_path($0);
	my $rootdir = shift;
	my $libdir = shift;
	my $filename = $File::Find::name;
	#print "$filename\n";
	if ($filename =~ m/.*\.o/ && not ($filename =~ m/.*third_party\/arduino.*/)){
		# link into a monilitic static library
		system("$rootdir/tools/gcc/gcc-arm-none-eabi/bin/arm-none-eabi-ar rvs $libdir/liblinkit.a $rootdir/$filename")
	}
}

sub build_system_libs
{
	(my $outpath)=@_;

	$outpath = "$outpath/mt7697/system/linkit_7697/libs";

	system("mkdir -p $outpath");

	my $wifi_prj="project/linkit_7697/apps/arduino/wifi/GCC/";

	system("cd $wifi_prj; make; cd -");

	# enlist all object files and link them
	my $cur_dir=abs_path(cwd());
	find(sub {
			\&process_obj($cur_dir, "$cur_dir/$outpath"),
		 },
		 ("$wifi_prj/Build/middleware", "$wifi_prj/Build/kernel", "$wifi_prj/Build/driver"));

	# copy system libraries
	my @libs_list=(
		"driver/chip/mt7687/lib/libhal_core_CM4_GCC.a",
		"driver/chip/mt7687/lib/libhal_protected_CM4_GCC.a",
		"driver/board/mt76x7_hdk/lib/libwifi.a",
		"middleware/MTK/minisupp/lib/libminisupp_wps.a",
		"middleware/MTK/minicli/lib/libminicli_CM4_GCC.a",
		"middleware/MTK/nvdm/lib/libnvdm_CM4_GCC.a",
		"kernel/service/lib/libkservice_CM4_MT7697_GCC.a");

	foreach $item (@libs_list) {
		system("cp $item $outpath");
	}

	system("cd $wifi_prj; make clean; cd -");
}

sub build_system_fw
{
	(my $outpath)=@_;

	my $bootloader_bin="project/linkit_7697/apps/bootloader/GCC/bootloader.bin";
	my $wifi_fw_bin="driver/chip/mt7687/wifi_n9/WIFI_RAM_CODE_MT76X7_in_flash.bin";
		
	$outpath = "$outpath/mt7697/system/linkit_7697/firmwares";
	system("mkdir -p $outpath");

	system("cp -r $bootloader_bin $outpath/mt7697_bootloader.bin");
	system("cp -r $wifi_fw_bin $outpath");
}

sub build_flash_tool
{
	(my $outpath)=@_;

	my $flash_tool="middleware/third_party/arduino/hardware/tools/mt7687/flash_tool";

	$outpath = "$outpath/";
	system("mkdir -p $outpath");

	system("cp -r $flash_tool $outpath/");
}


sub main
{
	my $outpath;
	my $pkg_ver;
	my $show_help;

	Getopt::Long::GetOptions(
		'o=s'		=> \$outpath,
		'v=s'		=> \$pkg_ver,
		'help'		=> \$show_help,
	);


	if (defined($show_help)) {
		print("USEAGE:
			-o=xxx		Indicate the output path
			-v=xxx		Indicate the package version\n");

		exit(0);
	}

	if (defined($outpath)) {
		if (! -d $outpath) {
			print("The outpath:$outpath is not exist.\n");
			exit(1);
		}
	} else {
		$outpath = "/tmp/";
	}

	$outpath =~ s/\/+$//;

	print("The outpath: $outpath\n");

	$pkg_ver="0.1.0" if (not defined($pkg_ver));
	print("The pgk_ver: $pkg_ver\n");

	# $pkg_name = "mt7697";
	
	# Pablo Test build libs
	&build_base_pkg($outpath);
	&build_system_src($outpath);
	&build_system_libs($outpath);
	&build_system_fw($outpath);

	&build_flash_tool($outpath);
}

&main();
