<?php

/**
 * @file
 * Contains rules integration for the path module needed during evaluation.
 *
 * @addtogroup rules
 * @{
 */

/**
 * Action implementation: Path alias.
 */
function rules_action_path_alias($source, $alias, $langcode = LANGUAGE_NONE) {
  if (!$alias) {
    path_delete(array('source' => $source, 'language' => $langcode));
  }
  elseif (!$source) {
    path_delete(array('alias' => $alias, 'language' => $langcode));
  }
  // Only set the alias if the alias is not taken yet.
  elseif (!path_load(array('alias' => $alias, 'language' => $langcode))) {
    // Update the existing path or create a new one.
    if ($path = path_load(array('source' => $source, 'language' => $langcode))) {
      $path['alias'] = $alias;
    }
    else {
      $path = array('source' => $source, 'alias' => $alias, 'language' => $langcode);
    }
    path_save($path);
  }
  else {
    rules_log('The configured alias %alias already exists. Aborting.', array('%alias' => $alias));
  }
}

/**
 * Action Implementation: Set the URL alias for a node.
 */
function rules_action_node_path_alias($node, $alias) {
  $langcode = isset($node->langcode) ? $node->langcode : LANGUAGE_NONE;
  // Only set the alias if the alias is not taken yet.
  if (($path = path_load(array('alias' => $alias, 'language' => $langcode))) && (empty($node->path['pid']) || $node->path['pid'] != $path['pid'])) {
    rules_log('The configured alias %alias already exists. Aborting.', array('%alias' => $alias));
    return FALSE;
  }
  $node->path['alias'] = $alias;
}

/**
 * Action Implementation: Set the URL alias for a node.
 */
function rules_action_taxonomy_term_path_alias($term, $alias) {
  // Only set the alias if the alias is not taken yet.
  if (($path = path_load(array('alias' => $alias, 'language' => LANGUAGE_NONE))) && (empty($term->path['pid']) || $term->path['pid'] != $path['pid'])) {
    rules_log('The configured alias %alias already exists. Aborting.', array('%alias' => $alias));
    return FALSE;
  }
  $term->path['alias'] = $alias;
}

/**
 * Condition implementation: Check if the path has an alias.
 */
function rules_condition_path_has_alias($source, $langcode = LANGUAGE_NONE) {
  return (bool) backdrop_lookup_path('alias', $source, $langcode);
}

/**
 * Condition implementation: Check if the URL alias exists.
 */
function rules_condition_path_alias_exists($alias, $langcode = LANGUAGE_NONE) {
  return (bool) backdrop_lookup_path('source', $alias, $langcode);
}

/**
 * Cleans the given path by replacing non ASCII characters with the replacment character.
 *
 * Path cleaning may be adapted by overriding the configuration variables
 * @code rules_clean_path @endcode,
 * @code rules_path_replacement_char @endcode and
 * @code rules_path_transliteration @endcode
 * in the site's settings.php file.
 */
function rules_path_default_cleaning_method($path) {
  $replace = config_get('rules.settings','rules_path_replacement_char');
  if ($replace) {
    // If the transliteration module is enabled, transliterate the alias first.
    if (module_exists('transliteration') && config_get('rules.settings','rules_path_transliteration')) {
      $path = transliteration_get($path);
    }

    $array = config_get('rules.settings','rules_clean_path');
    $array[2] = $path;
    // Replace it and remove trailing and leading replacement characters.
    $output = trim(call_user_func_array('preg_replace', $array), $replace);

    if (config_get('rules.settings','rules_path_lower_case')) {
      $output = backdrop_strtolower($output);
    }
    return $output;
  }
  else {
    return $path;
  }
}

/**
 * Cleans the given string so it can be used as part of a URL path.
 */
function rules_clean_path($path) {
  $function = config_get('rules.settings','rules_path_cleaning_callback');
  if (!function_exists($function)) {
    rules_log('An invalid URL path cleaning callback has been configured. Falling back to the default cleaning method.', array(), RulesLog::WARN);
    $function = 'rules_path_default_cleaning_method';
  }
  return $function($path);
}

/**
 * CTools path cleaning callback.
 *
 * @see rules_admin_settings()
 */
function rules_path_clean_ctools($path) {
  // Make use of the CTools cleanstring implementation.
  ctools_include('cleanstring');
  $settings = array(
    'separator' => config_get('rules.settings','rules_path_replacement_char'),
    'transliterate' => module_exists('transliteration') && config_get('rules.settings','rules_path_transliteration'),
    'lower case' => config_get('rules.settings','rules_path_lower_case'),
  );
  return ctools_cleanstring($path, $settings);
}

/**
 * Pathauto path cleaning callback.
 *
 * @see rules_admin_settings()
 */
function rules_path_clean_pathauto($path) {
  module_load_include('inc', 'pathauto');
  return pathauto_cleanstring($path);
}

/**
 * @}
 */
