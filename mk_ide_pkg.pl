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
        "driver/chip/mt7687/src/common/include/",
		"driver/CMSIS/Device/MTK/mt7687/Include/",
		"driver/CMSIS/Include/",
		"kernel/service/inc",
		"kernel/rtos/FreeRTOS/Source/include/",
		"kernel/rtos/FreeRTOS/Source/portable/GCC/ARM_CM4F/",
		"middleware/third_party/lwip/src/include/",
		"middleware/third_party/lwip/ports/include/",
		"middleware/third_party/mbedtls/include",
		"middleware/third_party/mbedtls/configs",
		"middleware/third_party/httpclient/inc",
		"middleware/MTK/wifi_service/combo/inc",
		"middleware/MTK/bluetooth/inc/",
		"middleware/MTK/nvdm/inc/",
		"middleware/MTK/connsys/inc/",
        "middleware/MTK/fota/inc/",
		"prebuilt/middleware/MTK/nvdm/inc/",
		"prebuilt/middleware/MTK/bluetooth/inc/",
		"middleware/MTK/dhcpd/inc",
		);
    
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

sub create_linkit_lib {
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

	my $arduino_lib_prj="project/linkit7697_hdk/apps/arduino/arduino_lib/GCC/";

	system("cd $arduino_lib_prj; make; cd -");

	# enlist all non-app layer object files and link them into "liblinkit.a"
	my $cur_dir=abs_path(cwd());
	find(sub {
			\&create_linkit_lib($cur_dir, "$cur_dir/$outpath"),
		 },
		 ("$arduino_lib_prj/Build/middleware", "$arduino_lib_prj/Build/kernel", "$arduino_lib_prj/Build/driver"));

	# copy system libraries
	my @libs_list=(
		"prebuilt/driver/chip/mt7687/lib/libhal_core_CM4_GCC.a",
		"prebuilt/driver/chip/mt7687/lib/libhal_protected_CM4_GCC.a",
		"prebuilt/middleware/MTK/wifi_service/combo/lib/wifi_supp/libwifi.a",
		"prebuilt/middleware/MTK/minisupp/lib/libminisupp.a",
		"prebuilt/middleware/MTK/minicli/lib/libminicli.a",
		"prebuilt/middleware/MTK/nvdm/lib/libnvdm_CM4_GCC.a",
		"prebuilt/middleware/MTK/bluetooth/lib/libble.a",
		"prebuilt/middleware/MTK/bluetooth/lib/libble_multi_adv.a",
		"prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_7697.a",
	);

	foreach $item (@libs_list) {
		system("cp $item $outpath");
	}

	system("cd $arduino_lib_prj; make clean; cd -");
}

sub build_system_fw
{
	(my $outpath)=@_;

	my $bootloader_bin="project/linkit7697_hdk/apps/bootloader/GCC/bootloader.bin";
	my $wifi_fw_bin="prebuilt/driver/chip/mt7687/wifi_n9/WIFI_RAM_CODE_MT76X7_in_flash.bin";
		
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
	my $tool_ver;
	my $show_help;

	Getopt::Long::GetOptions(
		'o=s'		=> \$outpath,
		'v=s'		=> \$pkg_ver,
		't=s'		=> \$tool_ver,
		'help'		=> \$show_help,
	);


	if (defined($show_help)) {
		print("USEAGE:
			-o=xxx		Indicate the output path
			-v=xxx		Indicate the package version
			-t=xxx      Indicate the tool version\n");

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

	$tool_ver="1.3.0" if (not defined($tool_ver));
	print("The tool_ver: $tool_ver\n");

	
	# build packages
	&build_base_pkg($outpath);
	&build_system_src($outpath);
	&build_system_libs($outpath);
	&build_system_fw($outpath);
	&build_flash_tool($outpath);

	# pack into bz2
	system("cd $outpath; tar -cvjf mediatek_linkit_7697-$pkg_ver.tar.bz2 mt7697; cd -");
	system("cd $outpath; tar -cvjf mediatek_linkit_7697_flash_tool-$tool_ver.tar.bz2 flash_tool; cd -");

	# TODO: 
	my $package_json="middleware/third_party/arduino/build/package_mtk_linkit_7697_index.json";
	system("cp $package_json $outpath");

}

&main();
