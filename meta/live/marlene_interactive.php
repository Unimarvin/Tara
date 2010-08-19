<?php

  require 'resource/php/session.php';

  if ( ! isset($_SESSION["rules"]) )
  {
    $_SESSION["rules"] = "";
  }
  
  if ( isset($_REQUEST) )
  {
    if ( isset($_REQUEST["rules"]) )
    {
      $_SESSION["rules"] = $_REQUEST["rules"];
      unset($_REQUEST);
      header("Location: ".$_SERVER["PHP_SELF"]."#rules");
    }
  }

  require 'resource/php/files.php';

  include 'resource/php/console.php';
  include 'resource/php/dotimg.php';
  include 'resource/php/getnumber.php';

  header("Content-Type: text/html");
  echo '<?xml version="1.0" encoding="utf-8" ?>';

  $services = array("marlene/pro.owfn", "marlene/req.owfn");
  //$rules = array("marlene/diag.ar");

  
  $services = prepareFiles($services);
  $rules = createFile("diag.ar");
  $diagfile = current(createFile("live.txt"));
  
  $rulefile = current($rules);

  $handle = fopen($rulefile["residence"], "w+");
  fwrite($handle, stripslashes($_SESSION['rules']));
  fclose($handle);

  $fakecall = "marlene";
  $realcall = "marlene";
  $fakeresult = "";
  $first = true;

  foreach($services as $info)
  {
    $fakecall .= " ".$info["basename"];
    $realcall .= " ".$info["residence"];
    if ($first == true)
    {
      $first = false;
    }
    else
    {
      $fakeresult .= "_";
    }
    $fakeresult .= $info["filename"];
  }
  $fakeresult .= ".owfn"; 
  $realresult = $_SESSION["dir"]."/".$fakeresult;

  $fakecall .= " -o ".$fakeresult;
  $realcall .= " -o ".$realresult;

  $fakecall .= " -r ".$rulefile["basename"];
  $realcall .= " -r ".$rulefile["residence"];

  $realcall .= " --diagnosis --property=livelock --live=".$diagfile["residence"]." -v 2>&1";

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
  <link rel="stylesheet" type="text/css" href="resource/css/format.css" />
  <link rel="stylesheet" type="text/css" href="resource/css/console.css" />
  <link rel="shortcut icon" href="resource/favicon.ico" type="image/x-icon" />
  <link rel="icon" href="resource/favicon.ico" type="image/x-icon" />
  <title>Adapter Synthesis <?php echo $_SESSION["uid"]; ?></title>
  <script type="text/javascript" src="resource/js/jquery-1.2.6.pack.js"></script>
</head>

<body>
  <div id="container">
    <div id="content">
      <h1>Adapter Synthesis</h1>

      <h2>Input Service Models</h2>

      <?php
      foreach($services as $file)
      {
	// system("petri --removePorts -o dot ".$file["residence"]);
        // dotimg("in=".urlencode($file["residence"]).".dot&amp;thumbnail_size=400&amp;label=".urlencode($file["basename"]));
        drawImage($file["basename"]);
      }
      ?>


      <ul>
<?php
  foreach($services as $info)
  {
    echo "<li><a href=".$info["link"].">".$info["basename"]."</a></li>";
  }
?>
      </ul>

      <h2>Diagnosis</h2>

      <?php
        $call_result = console($fakecall, 'cd marlene; '.$realcall.' ');
      ?>
    <a name="rules"></a>
    <h3>Rules</h3>
    <div style="text-align:center;">
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="POST">
      
      <?php
        echo "<textarea style=\"text-align: center;\" name=\"rules\" cols=\"50\" rows=\"10\">".$_SESSION["rules"]."</textarea>";
      ?>
        <p><input type="submit" value=" Save Rules "><input type="reset" value=" Undo Changes "></p>
     </form>
     </div>
<script>
function toggle(infoid)
{
if ($("#more"+infoid).is(":hidden")) {
$("#more"+infoid).slideDown("slow");
} else {
$("#more"+infoid).hide();
}

}

</script>

     <div style="margin-left:auto; margin-right:auto;">
<!--     
       <table style="margin-left:auto; margin-right:auto; border-width: 2px; border-style: solid; border-color: black;">
          <tr>
            <td>DL</td>
            <td>pending</td>
            <td>&rarr;</td>
            <td>required</td>
            <td onClick="toggle(bla)">more</td>
          </tr>
          <tr>
            <td colspan="5">
               <div id="morebla" style="display:none;">
                  bla
               </div>
            </td>
          </tr>
       </table>
-->       
<?PHP
  $dll = file($diagfile["residence"]);
  foreach ($dll as $index => $line)
  {
    // echo $line;
    list($type, $finals, $pending, $required, $rules) = explode(";", $line);
print<<<END
   
       <table style="width: 80%; margin-left:auto; margin-right:auto; border-width: 2px; border-style: solid; border-color: black;">
          <tr>
END;
    echo "<td>$type</td>\n";
    echo "<td>$pending</td>\n";
    echo "<td>&rarr;</td>\n";
    echo "<td>$required</td>\n";
    echo "<td onClick='toggle($index)'>more</td>\n";
print<<<END
          </tr>
          <tr>
            <td colspan="5">
END;
    echo "<div id=\"more$index\" style=\"display:none\">";
    echo "<h3>Additional information</h3>";
    if ( strcmp(rtrim($finals), "") )
    {
    $finals = explode(",", $finals);
    foreach ( $finals as $net )
    {
      // print_r($services);
      reset($services);
      if ( ! strcmp($net,"net1.") )
      {
        $serv = current($services);
        echo "<p>Net <i>".$serv["basename"]."</i> is already in a final state.</p>";
      }
      else
      {
        $serv = next($services);
        echo "<p>Net <i>".$serv["basename"]."</i> is already in a final state.</p>";
      }
    }
    }
    // $rules, $finals
    echo "<h4>Previously applied rules</h4>";
    if ( ! strcmp(rtrim($rules), "") )
    {
      echo "<p>none.</p>";
    }
    else
    {
      $ruleindex = explode(";", $_SESSION["rules"]);
      // print_r($ruleindex);
      $rules = explode (",", $rules);
      foreach($rules as $rule)
      {
          echo "<p>".$ruleindex[$rule-1]."</p>";
      }
     
    }
    echo "</div>\n";
print<<<END
            </td>
          </tr>
       </table>
END;
  }
?>
     </div>
     
      <?php drawImage($fakeresult); ?>
      <ul>
<?php
    echo "<li><a href=".getLink($fakeresult).">".basename($fakeresult)."</a></li>";
?>
      </ul>

      <p>
      	<a href="./#marlene" title="back to reality">Back to live</a>
      </p>
    </div>
  </div>
</body>

</html>